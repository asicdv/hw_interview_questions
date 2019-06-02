//========================================================================== //
// Copyright (c) 2018, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//========================================================================== //

`include "libtb2.vh"
`include "libv2_pkg.vh"

`include "fsm_quicksort_pkg.vh"

module fsm_quicksort (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Unsorted                                                                //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   unsorted_vld
   , input                                   unsorted_sop
   , input                                   unsorted_eop
   , input     [fsm_quicksort_pkg::W-1:0]    unsorted_dat
   //
   , output logic                            unsorted_rdy

   //======================================================================== //
   //                                                                         //
   // Sorted                                                                  //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                            sorted_vld_r
   , output logic                            sorted_sop_r
   , output logic                            sorted_eop_r
   , output logic                            sorted_err_r
   , output logic [fsm_quicksort_pkg::W-1:0] sorted_dat_r

   //======================================================================== //
   //                                                                         //
   // Control                                                                 //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                            busy_r
);
  import fsm_quicksort_pkg::*;

  typedef struct packed {
    logic        vld;
    logic        sop;
    logic        eop;
    logic        err;
    bank_n_t     idx;
  } dequeue_momento_t;

  typedef struct packed {
    logic        is_dat_p;
    logic        is_dat_i;
    logic        is_dat_j;
  } sort_momento_t;

  bank_state_t   [BANK_N-1:0]           bank_state_r;
  bank_state_t   [BANK_N-1:0]           bank_state_w;
  logic          [BANK_N-1:0]           bank_state_en;
  //
  bank_n_d_t                            queue_idle;
  bank_n_d_t                            queue_ready;
  bank_n_d_t                            queue_sorted;
  //
  addr_t                                enqueue_idx_r;
  addr_t                                enqueue_idx_w;
  logic                                 enqueue_idx_en;
  //
  bank_n_d_t                            dequeue_ack;
  bank_n_d_t                            dequeue_sel;
  bank_n_d_t                            dequeue_vld;
  //
  addr_t                                dequeue_idx_r;
  addr_t                                dequeue_idx_w;
  logic                                 dequeue_idx_en;
  //
  logic                                 unsorted_rdy;
  //
  enqueue_fsm_t                         enqueue_fsm_r;
  enqueue_fsm_t                         enqueue_fsm_w;
  logic                                 enqueue_fsm_en;
  //
  sort_context_t                        sort_context_r;
  sort_context_t                        sort_context_w;  
  logic                                 sort_context_en;
  //
  bank_n_t                              enqueue_bank_idx_r;
  bank_n_t                              enqueue_bank_idx_w;
  logic                                 enqueue_bank_idx_en;
  //
  enqueue_fsm_t                         dequeue_fsm_r;
  enqueue_fsm_t                         dequeue_fsm_w;
  logic                                 dequeue_fsm_en;
  //
  bank_n_t                              sort_bank_idx_r;
  bank_n_t                              sort_bank_idx_w;
  logic                                 sort_bank_idx_en;
  //
  bank_n_t                              dequeue_bank_idx_r;
  bank_n_t                              dequeue_bank_idx_w;
  logic                                 dequeue_bank_idx_en;
  //
  bank_state_t                          enqueue_bank;
  logic                                 enqueue_bank_en;
  //
  bank_state_t                          dequeue_bank;
  logic                                 dequeue_bank_en;
  //
  bank_state_t                          sort_bank;
  logic                                 sort_bank_en;
  //
  `SPSRAM_SIGNALS(enqueue__, W, $clog2(N));
  `SPSRAM_SIGNALS(dequeue__, W, $clog2(N));
  `SPSRAM_SIGNALS(sort__, W, $clog2(N));
  //
  logic                                 sorted_vld_w;
  logic                                 sorted_sop_w;
  logic                                 sorted_eop_w;
  logic                                 sorted_err_w;
  w_t                                   sorted_dat_w;
  //
  bank_n_d_t                            sortqueue_sel;
  //
  logic  [BANK_N-1:0]                   spsram_bank__en;
  logic  [BANK_N-1:0]                   spsram_bank__wen;
  addr_t [BANK_N-1:0]                   spsram_bank__addr;
  w_t    [BANK_N-1:0]                   spsram_bank__din;
  w_t    [BANK_N-1:0]                   spsram_bank__dout;
  //
  dequeue_momento_t                     dequeue_momento_in;
  dequeue_momento_t                     dequeue_momento_out_r;
  //
  logic                                 stack__cmd_vld;
  logic                                 stack__cmd_push;
  logic                                 stack__cmd_clr;
  sort_context_t                        stack__cmd_push_dat;
  sort_context_t                        stack__cmd_pop_dat_r;
  logic                                 stack__empty_r;
  logic                                 stack__full_r;
  //
  sort_momento_t                        sort_momento_in;
  sort_momento_t                        sort_momento_out_r;
  //
  
  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : queue_ctrl_PROC

      //
      queue_idle  = '0;
      for (int i = 0; i < BANK_N; i++)
        queue_idle [i]  = (bank_state_r [i].status == BANK_IDLE);

      //
      queue_ready  = '0;
      for (int i = 0; i < BANK_N; i++)
        queue_ready [i]  = (bank_state_r [i].status == BANK_READY);

      //
      queue_sorted       = '0;
      for (int i = 0; i < BANK_N; i++)
        queue_sorted [i]  = (bank_state_r [i].status == BANK_SORTED);

    end // block: queue_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : enqueue_fsm_PROC

      //
      enqueue__en          = '0;
      enqueue__wen         = '0;
      enqueue__addr        = '0;
      enqueue__din         = unsorted_dat;

      //
      enqueue_bank_idx_en  = '0;
      enqueue_bank_idx_w   = enqueue_bank_idx_r + 1'b1;

      //
      enqueue_bank         = '0;
      enqueue_bank_en      = '0;

      //
      enqueue_fsm_w        = enqueue_fsm_r;

      // verilator lint_off CASEINCOMPLETE
      unique case (enqueue_fsm_r)

        ENQUEUE_FSM_IDLE: begin

          if (unsorted_vld) begin
            enqueue__en          = 'b1;
            enqueue__wen         = 'b1;
            enqueue__addr        = '0;

            //
            enqueue_bank_en      = '1;
            enqueue_bank         = 0;
            enqueue_bank.status  = unsorted_eop ? BANK_READY : BANK_LOADING;

            //
            if (!unsorted_eop)
              enqueue_fsm_w  = ENQUEUE_FSM_LOAD;
            else
              enqueue_bank_idx_en  = 'b1;
          end
        end // case: ENQUEUE_FSM_IDLE

        ENQUEUE_FSM_LOAD: begin

          if (unsorted_vld) begin
            enqueue__en    = 'b1;
            enqueue__wen   = 'b1;
            enqueue__addr  = addr_t'(enqueue_idx_r);

            if (unsorted_eop) begin
              enqueue_bank_idx_en  = 'b1;

              //
              enqueue_bank         = '0;
              enqueue_bank.status  = BANK_READY;
              enqueue_bank.n       = {1'b0, enqueue_idx_r};
              enqueue_bank_en      = '1;

              //              
              enqueue_fsm_w        = ENQUEUE_FSM_IDLE;
            end
            
          end

        end // case: ENQUEUE_FSM_LOAD

      endcase // unique case (enqueue_fsm_r)
      // verilator lint_on CASEINCOMPLETE

      //
      unsorted_rdy    = queue_idle [enqueue_bank_idx_r];

      //
      enqueue_fsm_en  = (enqueue_fsm_r [ENQUEUE_FSM_BUSY_B] |
                         enqueue_fsm_w [ENQUEUE_FSM_BUSY_B]);

      //
      enqueue_idx_en  = enqueue_fsm_en;

      //
      unique case (enqueue_fsm_r)
        ENQUEUE_FSM_IDLE: enqueue_idx_w  = 'b1;
        default:          enqueue_idx_w  = enqueue_idx_r + 'b1;
      endcase // unique case (enqueue_fsm_r)

    end // block: enqueue_fsm_PROC

  // ------------------------------------------------------------------------ //
  //
  // Algorithm:
  //
  // function partition (A, lo, hi) is
  //   pivot := A[hi];
  //   i := lo;
  //   for j := lo to hi - 1 do
  //     if A[j] < pivot then
  //       swap A[i] with A[j];
  //       i := i + 1;
  //   swap A[i] with A[hi];
  //   return i;
  //
  // Pseudo-code:
  //
  // PROC PARTITION(A, lo, hi):
  //   0: P <- A[hi]; i <- lo; j <- lo;
  //   1: if (j == hi): goto 12;
  //   2: J <- A[j];
  //   3: WAIT;
  //   4: if (P <= J): goto 10;
  //   5: I <- A[i];
  //   6: WAIT;
  //   7: A[i] <- J;
  //   8: A[j] <- I;
  //   9: i <- i + 1;
  //  10: j <- j + 1;
  //  11: goto 1;
  //  12: I <- A[i];
  //  13: J <- A[hi];
  //  14: WAIT;
  //  15: A[hi] <- I;
  //  16: A[i] <- J;
  //  17: ret i;
  //
  always_comb
    begin : sort_fsm_PROC

      //
      sort__en             = '0;
      sort__wen            = '0;
      sort__addr           = '0;
      sort__din            = '0;

      //
      stack__cmd_vld       = '0;
      stack__cmd_push      = '0;
      stack__cmd_push_dat  = sort_context_r;
      stack__cmd_clr       = '0;

      //
      sort_momento_in      = '0;

      //
      sort_bank_idx_en     = '0;
      sort_bank_idx_w      = sort_bank_idx_r + 'b1;

      //
      sort_context_w       = sort_context_r;

      //
      sort_bank_en         = 'b0;
      sort_bank            = '0;

      // verilator lint_off CASEINCOMPLETE
      unique0 case (sort_context_r.state)
        SORT_FSM_IDLE: begin

          if (queue_ready [sort_bank_idx_r]) begin
            sort_context_w.lo     = '0;
            sort_context_w.hi     = bank_state_r [sort_bank_idx_r].n;

            stack__cmd_vld        = 'b1;
            stack__cmd_clr        = 'b1;

            sort_context_w.state  = SORT_FSM_START;
          end
        end
        SORT_FSM_START: begin
          if (sort_context_r.lo < sort_context_r.hi)
            sort_context_w.state  = SORT_FSM_PARTITION_0;
          else
            sort_context_w.state  = SORT_FSM_DONE;
        end
        SORT_FSM_PARTITION_0: begin
          sort__en                  = 'b1;
          sort__wen                 = 'b0;
          sort__addr                = addr_t'(sort_context_r.hi);

          sort_momento_in.is_dat_p  = 'b1;

          sort_context_w.i          = sort_context_r.lo;
          sort_context_w.j          = sort_context_r.lo;
          
          sort_context_w.state      = SORT_FSM_PARTITION_1;
        end
        SORT_FSM_PARTITION_1: begin
          if (sort_context_r.j == sort_context_r.hi)
            sort_context_w.state  = SORT_FSM_PARTITION_12;
          else
            sort_context_w.state  = SORT_FSM_PARTITION_2;
        end
        SORT_FSM_PARTITION_2: begin
          sort__en                  = 'b1;
          sort__wen                 = 'b0;
          sort__addr                = addr_t'(sort_context_r.j);

          sort_momento_in.is_dat_j  = 'b1;
          
          sort_context_w.state      = SORT_FSM_PARTITION_3;
        end
        SORT_FSM_PARTITION_3: begin
          sort_context_w.state  = SORT_FSM_PARTITION_4;
        end
        SORT_FSM_PARTITION_4: begin
          if (sort_context_r.dat_p <= sort_context_r.dat_j)
            sort_context_w.state  = SORT_FSM_PARTITION_10;
          else
            sort_context_w.state  = SORT_FSM_PARTITION_5;
        end
        SORT_FSM_PARTITION_5: begin
          sort__en                  = 'b1;
          sort__wen                 = 'b0;
          sort__addr                = addr_t'(sort_context_r.i);
          
          sort_momento_in.is_dat_i  = 'b1;

          sort_context_w.state      = SORT_FSM_PARTITION_6;
        end
        SORT_FSM_PARTITION_6: begin
          sort_context_w.state  = SORT_FSM_PARTITION_7;
        end
        SORT_FSM_PARTITION_7: begin
          sort__en              = 'b1;
          sort__wen             = 'b1;
          sort__addr            = addr_t'(sort_context_r.i);
          sort__din             = sort_context_r.dat_j;

          sort_context_w.state  = SORT_FSM_PARTITION_8;
        end
        SORT_FSM_PARTITION_8: begin
          sort__en              = 'b1;
          sort__wen             = 'b1;
          sort__addr            = addr_t'(sort_context_r.j);
          sort__din             = sort_context_r.dat_i;

          sort_context_w.state      = SORT_FSM_PARTITION_9;
        end
        SORT_FSM_PARTITION_9: begin
          sort_context_w.i      = sort_context_r.i + 'b1;
          
          sort_context_w.state  = SORT_FSM_PARTITION_10;
        end
        SORT_FSM_PARTITION_10: begin
          sort_context_w.j      = sort_context_r.j + 'b1;

          sort_context_w.state  = SORT_FSM_PARTITION_11;
        end
        SORT_FSM_PARTITION_11: begin
          sort_context_w.state  = SORT_FSM_PARTITION_1;
        end
        SORT_FSM_PARTITION_12: begin
          sort__en                  = 'b1;
          sort__wen                 = 'b0;
          sort__addr                = addr_t'(sort_context_r.i);
          
          sort_momento_in.is_dat_i  = 'b1;
          
          sort_context_w.state      = SORT_FSM_PARTITION_13;
        end
        SORT_FSM_PARTITION_13: begin
          sort__en                  = 'b1;
          sort__wen                 = 'b0;
          sort__addr                = addr_t'(sort_context_r.hi);
          
          sort_momento_in.is_dat_j  = 'b1;

          sort_context_w.state      = SORT_FSM_PARTITION_14;
        end
        SORT_FSM_PARTITION_14: begin
          sort_context_w.state      = SORT_FSM_PARTITION_15;
        end
        SORT_FSM_PARTITION_15: begin
          sort__en              = 'b1;
          sort__wen             = 'b1;
          sort__addr            = addr_t'(sort_context_r.hi);
          sort__din             = sort_context_r.dat_i;
          
          sort_context_w.state  = SORT_FSM_PARTITION_16;
        end
        SORT_FSM_PARTITION_16: begin
          sort__en              = 'b1;
          sort__wen             = 'b1;
          sort__addr            = addr_t'(sort_context_r.i);
          sort__din             = sort_context_r.dat_j;

          sort_context_w.state  = SORT_FSM_QUICKSORT_LO;
        end
        SORT_FSM_QUICKSORT_LO: begin
          if (!stack__full_r) begin
            stack__cmd_vld             = 'b1;
            stack__cmd_push            = 'b1;
            stack__cmd_push_dat.state  = SORT_FSM_QUICKSORT_HI;
            
            sort_context_w.hi          = sort_context_r.i - 'b1;
            
            sort_context_w.state       = SORT_FSM_START;
          end else begin
            stack__cmd_vld        = 'b1;
            stack__cmd_clr        = 'b1;

            sort_context_w.error  = 'b1;
            sort_context_w.state  = SORT_FSM_DONE;
          end
        end
        SORT_FSM_QUICKSORT_HI: begin
          if (!stack__full_r) begin
            stack__cmd_vld             = 'b1;
            stack__cmd_push            = 'b1;
            stack__cmd_push_dat.state  = SORT_FSM_DONE;
          
            sort_context_w.lo          = sort_context_r.i + 'b1;
            sort_context_w.state       = SORT_FSM_START;
          end else begin
            stack__cmd_vld        = 'b1;
            stack__cmd_clr        = 'b1;
            
            sort_context_w.error  = 'b1;
            sort_context_w.state  = SORT_FSM_DONE;
          end
        end
        SORT_FSM_DONE: begin
          if (stack__empty_r) begin
            sort_bank_idx_en      = 'b1;

            sort_bank_en          = 'b1;
            sort_bank             = bank_state_r [sort_bank_idx_r];
            sort_bank.status      = BANK_SORTED;
            sort_bank.error       = sort_context_r.error;
            
            sort_context_w.state  = SORT_FSM_IDLE;
          end else begin
            stack__cmd_vld   = 'b1;
            stack__cmd_push  = 'b0;

            sort_context_w   = stack__cmd_pop_dat_r;
          end
        end
      endcase // unique0 case (sort_context_r.state)
      // verilator lint_on CASEINCOMPLETE

      //
      unique0 case (1'b1)
        sort_momento_out_r.is_dat_p: begin
          sort_context_w.dat_p  = spsram_bank__dout [sort_bank_idx_r];
        end
        sort_momento_out_r.is_dat_i: begin
          sort_context_w.dat_i  = spsram_bank__dout [sort_bank_idx_r];
        end
        sort_momento_out_r.is_dat_j: begin
          sort_context_w.dat_j  = spsram_bank__dout [sort_bank_idx_r];
        end
      endcase // unique0 case (1'b1)

      //
      sort_context_en =   (|sort_momento_out_r)
                        | sort_context_w.state [SORT_FSM_BUSY_B]
                        | sort_context_r.state [SORT_FSM_BUSY_B]
                      ;

    end // block: sort_fsm_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : dequeue_fsm_PROC

      //
      dequeue__en             = 'b0;
      dequeue__wen            = 'b0;
      dequeue__addr           = 'b0;
      dequeue__din            = 'b0;

      //
      dequeue_bank_idx_en     = '0;
      dequeue_bank_idx_w      = dequeue_bank_idx_r + 'b1;

      //
      dequeue_ack             = 'b1;

      //
      dequeue_bank            = 'b0;
      dequeue_bank_en         = 'b0;

      //
      dequeue_momento_in.vld  = '0;
      dequeue_momento_in.sop  = '0;
      dequeue_momento_in.eop  = '0;
      dequeue_momento_in.err  = '0;
      dequeue_momento_in.idx  = dequeue_bank_idx_r;

      sorted_err_w            = 'b0;
      
      //
      dequeue_fsm_w           = dequeue_fsm_r;

      // verilator lint_off CASEINCOMPLETE
      unique case (dequeue_fsm_r)

        DEQUEUE_FSM_IDLE: begin

          if (queue_sorted [dequeue_bank_idx_r]) begin
            bank_state_t st         = bank_state_r [dequeue_bank_idx_r];
            
            dequeue__en             = 'b1;
            dequeue__addr           = 'b0;

            //
            dequeue_momento_in.vld  = '1;
            dequeue_momento_in.sop  = '1;
            dequeue_momento_in.err  = st.error;

            dequeue_bank_en         = 'b1;
            dequeue_bank            = st;
            
            if (st.n == '0) begin
              dequeue_momento_in.eop  = '1;

              dequeue_bank.status     = BANK_IDLE;
            end else begin
              dequeue_momento_in.eop  = '0;
              
              dequeue_bank.status    = BANK_UNLOADING;
              dequeue_fsm_w          = DEQUEUE_FSM_EMIT;
            end
          end
        end

        DEQUEUE_FSM_EMIT: begin
          bank_state_t st         = bank_state_r [dequeue_bank_idx_r];
          
          dequeue__en             = 'b1;
          dequeue__addr           = dequeue_idx_r;

          //
          dequeue_momento_in.vld  = '1;
          dequeue_momento_in.sop  = '0;
          dequeue_momento_in.eop  = '0;
          dequeue_momento_in.err  = st.error;

          if (dequeue_idx_r == addr_t'(st.n)) begin
            dequeue_bank_idx_en     = 'b1;

            //
            dequeue_momento_in.eop  = '1;

            //
            dequeue_bank_en         = 'b1;
            dequeue_bank            = st;
            dequeue_bank.status     = BANK_IDLE;

            dequeue_fsm_w           = DEQUEUE_FSM_IDLE;
          end
          
        end // case: DEQUEUE_FSM_EMIT
      endcase // unique case (dequeue_fsm_r)
      // verilator lint_on CASEINCOMPLETE

      //
      dequeue_fsm_en  = (dequeue_fsm_w [DEQUEUE_FSM_BUSY_B] |
                         dequeue_fsm_r [DEQUEUE_FSM_BUSY_B]);

      //
      dequeue_idx_en  = dequeue_fsm_en;

      //
      unique case (dequeue_fsm_r)
        DEQUEUE_FSM_IDLE: dequeue_idx_w  = 'b1;
        default:          dequeue_idx_w  = dequeue_idx_r + 'b1;
      endcase // unique case (dequeue_fsm_r)

    end // block: dequeue_fsm_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : bank_PROC

      for (int i = 0; i < BANK_N; i++) begin

        //
        bank_state_en [i]  = '0;
        bank_state_w [i]   = '0;

        if (bank_n_t'(i)   == enqueue_bank_idx_r) begin
          bank_state_en [i]  |= enqueue_bank_en;
          bank_state_w [i]   |= enqueue_bank;
        end
        if (bank_n_t'(i)   == sort_bank_idx_r) begin
          bank_state_en [i]  |= sort_bank_en;
          bank_state_w [i]   |= sort_bank;
        end
        if (bank_n_t'(i)   == dequeue_bank_idx_r) begin
          bank_state_en [i]  |= dequeue_bank_en;
          bank_state_w [i]   |= dequeue_bank;
        end

      end

    end // block: bank_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : spsram_PROC

      for (int i = 0; i < BANK_N; i++) begin
        
        spsram_bank__en [i]    = '0;
        spsram_bank__wen [i]   = '0;
        spsram_bank__addr [i]  = '0;
        spsram_bank__din [i]   = '0;

        if (bank_n_t'(i) == enqueue_bank_idx_r) begin
          spsram_bank__en [i]    |= enqueue__en;
          spsram_bank__wen [i]   |= enqueue__wen;
          spsram_bank__addr [i]  |= enqueue__addr;
          spsram_bank__din [i]   |= enqueue__din;
        end
        if (bank_n_t'(i) == sort_bank_idx_r) begin
          spsram_bank__en [i]    |= sort__en;
          spsram_bank__wen [i]   |= sort__wen;
          spsram_bank__addr [i]  |= sort__addr;
          spsram_bank__din [i]   |= sort__din;
        end
        if (bank_n_t'(i) == dequeue_bank_idx_r) begin
          spsram_bank__en [i]    |= dequeue__en;
          spsram_bank__wen [i]   |= dequeue__wen;
          spsram_bank__addr [i]  |= dequeue__addr;
          spsram_bank__din [i]   |= dequeue__din;
        end

      end // for (int i = 0; i < BANK_N; i++)

    end // block: spsram_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : sorted_out_PROC

      //
      sorted_vld_w  = dequeue_momento_out_r.vld;
      sorted_sop_w  = dequeue_momento_out_r.sop;
      sorted_eop_w  = dequeue_momento_out_r.eop;
      sorted_err_w  = dequeue_momento_out_r.err;
      sorted_dat_w  = spsram_bank__dout [dequeue_momento_out_r.idx];

    end // block: sorted_out_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      enqueue_fsm_r <= ENQUEUE_FSM_IDLE;
    else if (enqueue_fsm_en)
      enqueue_fsm_r <= enqueue_fsm_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      sort_context_r <= '{state:SORT_FSM_IDLE, default:'d0};
    else if (sort_context_en)
      sort_context_r <= sort_context_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      dequeue_fsm_r <= DEQUEUE_FSM_IDLE;
    else if (dequeue_fsm_en)
      dequeue_fsm_r <= dequeue_fsm_w;
      
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      enqueue_idx_r <= 'b0;
    else if (enqueue_idx_en)
      enqueue_idx_r <= enqueue_idx_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      dequeue_idx_r <= 'b0;
    else if (dequeue_idx_en)
      dequeue_idx_r <= dequeue_idx_w;

  // ------------------------------------------------------------------------ //
  // TODO - rename to SCOREBOARD
  always_ff @(posedge clk) begin : bank_reg_PROC
    if (rst) begin
      for (int i = 0; i < BANK_N; i++)
        bank_state_r [i] <= '{status:BANK_IDLE, default:'0};
    end else begin
      for (int i = 0; i < BANK_N; i++)
        if (bank_state_en [i])
          bank_state_r [i] <= bank_state_w [i];
    end
  end // block: bank_reg_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      enqueue_bank_idx_r <= '0;
    else if (enqueue_bank_idx_en)
      enqueue_bank_idx_r <= enqueue_bank_idx_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      dequeue_bank_idx_r <= '0;
    else if (dequeue_bank_idx_en)
      dequeue_bank_idx_r <= dequeue_bank_idx_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      sort_bank_idx_r <= '0;
    else if (sort_bank_idx_en)
      sort_bank_idx_r <= sort_bank_idx_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      sorted_vld_r <= 'b0;
    else
      sorted_vld_r <= sorted_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (sorted_vld_w) begin
      sorted_sop_r  = sorted_sop_w;
      sorted_eop_r  = sorted_eop_w;
      sorted_err_r  = sorted_err_w;
      sorted_dat_r  = sorted_dat_w;
    end

  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  stack #(.W(SORT_CONTEXT_W), .N(16)) u_stack (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .cmd_vld           (stack__cmd_vld     )
    , .cmd_push          (stack__cmd_push    )
    , .cmd_push_dat      (stack__cmd_push_dat)
    , .cmd_clr           (stack__cmd_clr     )
    //
    , .cmd_pop_dat_r     (stack__cmd_pop_dat_r)
    //
    , .empty_r           (stack__empty_r     )
    , .full_r            (stack__full_r      )
  );

  // ------------------------------------------------------------------------ //
  //
  delay_pipe #(.W($bits(dequeue_momento_t)), .N(1)) u_dequeue_momento_delay_pipe (
    //
      .clk               (clk                     )
    , .rst               (rst                     )
    //
    , .in                (dequeue_momento_in      )
    , .out_r             (dequeue_momento_out_r   )
  );

  // ------------------------------------------------------------------------ //
  //
  delay_pipe #(.W($bits(sort_momento_t)), .N(1)) u_sort_momento_delay_pipe (
    //
      .clk               (clk                     )
    , .rst               (rst                     )
    //
    , .in                (sort_momento_in         )
    , .out_r             (sort_momento_out_r      )
  );

  // ------------------------------------------------------------------------ //
  //
  generate for (genvar g = 0; g < BANK_N; g++) begin
  
    spsram #(.W(W), .N(N)) u_spsram_bank (
      //
        .clk           (clk                       )
      //
      , .en            (spsram_bank__en [g]       )
      , .wen           (spsram_bank__wen [g]      )
      , .addr          (spsram_bank__addr [g]     )
      , .din           (spsram_bank__din [g]      )
      //
      , .dout          (spsram_bank__dout [g]     )
    );

  end endgenerate

endmodule // fsm_quicksort
