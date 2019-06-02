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

`include "vert_ucode_quicksort_pkg.vh"

module vert_ucode_quicksort (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst

   //======================================================================== //
   //                                                                         //
   // Unsorted                                                                //
   //                                                                         //
   //======================================================================== //

   //
   , input                                        unsorted_vld
   , input                                        unsorted_sop
   , input                                        unsorted_eop
   , input     [vert_ucode_quicksort_pkg::W-1:0]  unsorted_dat
   //
   , output logic                                 unsorted_rdy

   //======================================================================== //
   //                                                                         //
   // Sorted                                                                  //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                                 sorted_vld_r
   , output logic                                 sorted_sop_r
   , output logic                                 sorted_eop_r
   , output logic                                 sorted_err_r
   , output logic [vert_ucode_quicksort_pkg::W-1:0] sorted_dat_r

   //======================================================================== //
   //                                                                         //
   // Control                                                                 //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                            busy_r
);
  import vert_ucode_quicksort_pkg::*;

  typedef struct packed {
    logic        vld;
    logic        sop;
    logic        eop;
    logic        err;
    bank_n_t     idx;
  } dequeue_momento_t;

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
  //
  logic                                 sort__en /* verilator public */;
  logic                                 sort__wen /* verilator public */;
  addr_t                                sort__addr /* verilator public */;
  w_t                                   sort__din /* verilator public */;
  w_t                                   sort__dout /* verilator public */;
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
  w_t                                   stack__cmd_push_dat;
  w_t                                   stack__cmd_pop_dat_r;
  logic                                 stack__empty_r;
  logic                                 stack__full_r;
  //
  logic                                 sort_momento_in;
  logic                                 sort_momento_out_r;
  //
  w_t                                   adder__y;
  logic                                 adder__cout;
  w_t                                   adder__a;
  w_t                                   adder__b_pre;
  w_t                                   adder__b;
  logic                                 adder__cin;
  //
  logic                                 ar_flag_en;
  //
  logic                                 ar_flag_z_w;
  logic                                 ar_flag_n_w;
  logic                                 ar_flag_c_w;
  //
  logic                                 ar_flag_z_r /* verilator public */;
  logic                                 ar_flag_n_r /* verilator public */;
  logic                                 ar_flag_c_r /* verilator public */;
  //
  reg_t [1:0]                           da_rf__ra;
  logic [1:0]                           da_rf__ren;
  w_t   [1:0]                           da_rf__rdata;
  //
  reg_t                                 da_rf__wa_w /* verilator public */;
  logic                                 da_rf__wen_w /* verilator public */;
  w_t                                   da_rf__wdata_w /* verilator public */;
  //
  reg_t                                 da_rf__wa_r;
  logic                                 da_rf__wen_r;
  w_t                                   da_rf__wdata_r;
  //
  logic                                 fa_kill;
  logic                                 fa_valid;
  logic                                 fa_adv;
  logic                                 fa_pass;
  pc_t                                  fa_pc_r;
  pc_t                                  fa_pc_w;
  logic                                 fa_pc_en;
  //
  logic                                 da_en;
  logic                                 da_stall;
  logic                                 da_adv /* verilator public */;
  logic                                 da_mem_op_issue;
  logic                                 da_valid_w;
  logic                                 da_valid_r;
  logic                                 da_cc_hit;
  logic                                 da_taken_branch;
  logic                                 da_taken_ret;
  logic                                 da_taken_call;
  logic                                 da_ld_stall_r;
  logic                                 da_ld_stall_w;
  pc_t                                  da_pc_w;
  pc_t                                  da_pc_r /* verilator public */;
  //
  logic                                 da_src0_is_wrbk;
  w_t                                   da_src0;
  logic                                 da_src1_is_wrbk;
  w_t                                   da_src1;
  //
  ucode_t                               da_ucode;
  //
  inst_t                                da_inst_r /* verilator public */;
  inst_t                                da_inst_w;
  
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
  // function partition (lo, hi) is
  //   pivot := A[hi];
  //   i := lo;
  //   for j := lo to hi - 1 do
  //     if A[j] < pivot then
  //       swap A[i] with A[j];
  //       i := i + 1;
  //   swap A[i] with A[hi];
  //   return i;
  //
  // function quicksort (lo, hi) is
  //   if lo < hi:
  //     p := partition(lo, hi)
  //     quicksort(lo, p - 1)
  //     quicksort(p + 1, hi)
  //
  //        XXXX_YYYY_YYYY_YYYY
  //  -------------------------
  //    NOP 0000_XXXX_XXXX_XXXX
  //
  //    Jcc 0001_XXcc_AAAA_AAAA
  //
  //     00 - "" Unconditional
  //     01 - "EQ" Equal
  //     10 - "GT" Greather-Than
  //     11 - "LE" Less-Than or Equal
  //
  //   PUSH 0010_0XXX_XXXX_Xuuu
  //    POP 0010_1rrr_XXXX_XXXX
  //
  //     LD 0100_0rrr_XXXX_Xuuu
  //     ST 0100_1XXX_Xsss_Xuuu
  //
  //    MOV 0110_0rrr_XXXX_0uuu
  //   MOVI 0110_0rrr_XXXX_1iii
  //   MOVS 0110_1rrr_XXXX_XSSS
  //
  //    ADD 0111_0rrr_Wsss_0uuu
  //   ADDI 0111_0rrr_Wsss_1iii
  //    SUB 0111_1rrr_Wsss_0uuu
  //   SUBI 0111_1rrr_Wsss_1iii
  //
  //   CALL 1100_0XXX_AAAA_AAAA
  //    RET 1100_1XXX_XXXX_XXXX
  //
  //   WAIT 1111_0XXX_XXXX_XXXX
  //   EMIT 1111_1XXX_XXXX_XXXX
  //
  // PROC RESET:
  //   __reset     : J __start        ; PROC RESET:
  //
  // PROC PARTITION:
  //   __part      : PUSH R2          ;
  //               : PUSH R3          ;
  //               : PUSH R4          ;
  //               : PUSH R5          ;
  //               : PUSH R6          ;
  //               : LD R2, [R1]      ; pivot <- A[hi];
  //               : MOV R3, R0       ; i <- lo
  //               : MOV R4, R0       ; j <- lo
  //   __loop_start: SUB 0, R1, R4    ;
  //               : JEQ __end        ; if (j == hi) goto __end
  //               : LD R5, [R4]      ; R5 <- A[j]
  //               : SUB.F 0, R5, R2  ;
  //               : JGT __end_loop   ; if ((A[j] - pivot) > 0) goto __end_of_loop
  //               : LD R6, [R3]      ; swap A[i] with A[j]
  //               : ST [R3], R5      ;
  //               : ST [R4], R6      ;
  //               : ADDI R3, R3, 1   ; i <- i + 1
  //   __end_loop  : ADDI R4, R4, 1   ; j <- j + 1
  //               : J __loop_start   ;
  //   __end       : LD R0, [R3]      ;
  //               : LD R1, [R4]      ;
  //               : ST [R3], R1      ;
  //               : ST [R4], R0      ;
  //               : MOV R0,R3        ; ret <- pivot
  //               : POP R6           ;
  //               : POP R5           ;
  //               : POP R4           ;
  //               : POP R3           ;
  //               : POP R2           ;
  //               : RET              ;
  //
  // PROC QUICKSORT:
  //   __qs        : PUSH BLINK       ;
  //               : PUSH R2          ;
  //               : PUSH R3          ;
  //               : PUSH R4          ;
  //               : MOV R2, R0       ; R2 <- LO
  //               : MOV R4, R1       ; R4 <- HI
  //               : SUB.F 0, R1, R0  ;
  //               : JGT __qs_end     ; if ((hi - lo) <= 0) goto __end;
  //               : CALL PARTITION   ; R0 <- partition(lo, hi);
  //               : MOV R3, R0       ; R3 <- PIVOT
  //               : MOV R0, R2       ;
  //               : SUBI R1, R3, 1   ;
  //               : CALL QUICKSORT   ; quicksort(lo, p - 1);
  //               : ADDI R0, R3, 1   ;
  //               : MOV R1, R4       ;
  //               : CALL QUICKSORT   ; quicksort(p + 1, hi);
  //   __qs_end    : POP R4           ;
  //               : POP R3           ;
  //               : POP R2           ;
  //               : POP BLINK        ; 
  //               : RET              ; PC <- BLINK
  //
  //  PROC START:
  //   __start     : WAIT             ; wait until queue_ready == 1
  //
  //               : MOVI R0, 0       ;
  //               : MOVS R1, N       ; 
  //               : CALL __qs        ; call quicksort(A, lo, hi);
  //               : EMIT             ;
  //               : J __main         ; goto __main
  //
  `include "vert_ucode_quicksort_insts.vh"
  //
  localparam pc_t SYM_RESET  = 'd0;
  localparam pc_t SYM_START  = 'd32;
  localparam pc_t SYM_PARTITION  = 'd64;
  localparam pc_t SYM_QUICKSORT  = 'd96;

  always_comb
    begin : quicksort_prog_PROC

      //
      da_inst_w  = '0;
      da_pc_w    = fa_pc_r;

      // Control Store
      //
      // Implemented here as a simple lookup table, but in practise more
      // more efficiently realized as a ROM. Perhaps an FPGA synthesis
      // tool can automatically infer a ROM from this table, otherwise, the
      // microcode would need to be hand assembled and loaded as a HEX-file.
      
      case (fa_pc_r)
        //
        SYM_RESET          : inst_j(SYM_START);

        //
        SYM_PARTITION      : inst_push(R2);
        SYM_PARTITION +   1: inst_push(R3);
        SYM_PARTITION +   2: inst_push(R4);
        SYM_PARTITION +   3: inst_push(R5);
        SYM_PARTITION +   4: inst_push(R6);
        SYM_PARTITION +   5: inst_ld(R2, R1);
        SYM_PARTITION +   6: inst_mov(R3, R0);
        SYM_PARTITION +   7: inst_mov(R4, R0);
        SYM_PARTITION +   8: inst_sub(R0, R1, R4, .dst_en('b0));
        SYM_PARTITION +   9: inst_j(SYM_PARTITION + 19, .cc(EQ));
        SYM_PARTITION +  10: inst_ld(R5, R4);
        SYM_PARTITION +  11: inst_sub(R0, R5, R2, .dst_en('b0));
        SYM_PARTITION +  12: inst_j(SYM_PARTITION + 17, .cc(GT));
        SYM_PARTITION +  13: inst_ld(R6, R3);
        SYM_PARTITION +  14: inst_st(R3, R5);
        SYM_PARTITION +  15: inst_st(R4, R6);
        SYM_PARTITION +  16: inst_addi(R3, R3, 'b1);
        SYM_PARTITION +  17: inst_addi(R4, R4, 'b1);
        SYM_PARTITION +  18: inst_j(SYM_PARTITION + 8);
        SYM_PARTITION +  19: inst_ld(R0, R3);
        SYM_PARTITION +  20: inst_ld(R1, R4);
        SYM_PARTITION +  21: inst_st(R3, R1);
        SYM_PARTITION +  22: inst_st(R4, R0);
        SYM_PARTITION +  23: inst_mov(R0, R3);
        SYM_PARTITION +  24: inst_pop(R6);
        SYM_PARTITION +  25: inst_pop(R5);
        SYM_PARTITION +  26: inst_pop(R4);
        SYM_PARTITION +  27: inst_pop(R3);
        SYM_PARTITION +  28: inst_pop(R2);
        SYM_PARTITION +  29: inst_ret();

        //
        SYM_QUICKSORT      : inst_push(BLINK);
        SYM_QUICKSORT +   1: inst_push(R2);
        SYM_QUICKSORT +   2: inst_push(R3);
        SYM_QUICKSORT +   3: inst_push(R4);
        SYM_QUICKSORT +   4: inst_mov(R2, R0);
        SYM_QUICKSORT +   5: inst_mov(R4, R1);
        SYM_QUICKSORT +   6: inst_sub(R0, R0, R1, .dst_en('b0));
        SYM_QUICKSORT +   7: inst_j(SYM_QUICKSORT + 16, GT);
        SYM_QUICKSORT +   8: inst_call(SYM_PARTITION);
        SYM_QUICKSORT +   9: inst_mov(R3, R0);
        SYM_QUICKSORT +  10: inst_mov(R0, R2);
        SYM_QUICKSORT +  11: inst_subi(R1, R3, 'd1);
        SYM_QUICKSORT +  12: inst_call(SYM_QUICKSORT);
        SYM_QUICKSORT +  13: inst_addi(R0, R3, 'd1);
        SYM_QUICKSORT +  14: inst_mov(R1, R4);
        SYM_QUICKSORT +  15: inst_call(SYM_QUICKSORT);
        SYM_QUICKSORT +  16: inst_pop(R4);
        SYM_QUICKSORT +  17: inst_pop(R3);
        SYM_QUICKSORT +  18: inst_pop(R2);
        SYM_QUICKSORT +  19: inst_pop(BLINK);
        SYM_QUICKSORT +  20: inst_ret();

        //
        SYM_START          : inst_wait();
        SYM_START +       1: inst_movi(R0, '0);
        SYM_START +       2: inst_movs(R1, REG_N);
        SYM_START +       3: inst_call(SYM_QUICKSORT);
        SYM_START +       4: inst_emit();
        SYM_START +       5: inst_j(SYM_START);
        
        default:             inst_nop();
        
      endcase // case (fetch_pc_r)

    end // block: quicksort_prog_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : datapath_PROC

      //
      da_ucode  = decode(da_inst_r);

      //
      unique case (da_ucode.cc)
        EQ:      da_cc_hit  = ar_flag_z_r;
        GT:      da_cc_hit  = (~ar_flag_z_r) & (~ar_flag_n_r);
        LE:      da_cc_hit  = ar_flag_z_r | ar_flag_n_r;
        default: da_cc_hit  = 1'b1;
      endcase // unique case (da_ucode.cc)

      //
      case (da_ld_stall_r)
        1'b1:    da_ld_stall_w  = (~sort_momento_out_r);
        default: da_ld_stall_w  = da_valid_r & da_ucode.is_load;
      endcase // case (da_ld_stall_r)

      //
      priority case (1'b1)
        da_ucode.is_wait: da_stall  = (~queue_ready [sort_bank_idx_r]);
        default:          da_stall  = da_ld_stall_w;
      endcase // priority case (1'b1)
      
      //
      da_taken_branch  = da_valid_r & da_ucode.is_jump && da_cc_hit;
      da_taken_ret     = da_valid_r & da_ucode.is_ret;
      da_taken_call    = da_valid_r & da_ucode.is_call;

      //
      fa_kill          = da_taken_branch;
      fa_valid         = (~fa_kill);
      fa_pass          = fa_valid & (~fa_kill);
      fa_adv           = fa_pass & (~da_valid_r | ~da_stall);

      //
      da_valid_w       = fa_pass & (~da_stall) | (da_valid_r & da_stall);
      da_adv           = da_valid_r & (~da_stall);
      da_en            = (~da_stall);

      //
      da_rf__ra        = {da_ucode.src1, da_ucode.src0};

      //
      case (1'b1)
        da_ucode.is_store: da_mem_op_issue  = da_valid_r;
        da_ucode.is_load:  da_mem_op_issue  = da_valid_r & (~da_ld_stall_r);
        default:           da_mem_op_issue  = '0;
      endcase // case (1'b1)
      
      //
      da_src0_is_wrbk  = da_rf__wen_r & (da_rf__ra [0] == da_rf__wa_r);
      da_src1_is_wrbk  = da_rf__wen_r & (da_rf__ra [1] == da_rf__wa_r);

      //
      da_rf__ren [0]   = da_ucode.src0_en & (~da_src0_is_wrbk);
      da_rf__ren [1]   = da_ucode.src1_en & (~da_src1_is_wrbk);

    end // block: datapath_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : exe_PROC

      //
      da_src0   = da_src0_is_wrbk ? da_rf__wdata_r : da_rf__rdata [0];
      da_src1   = da_src1_is_wrbk ? da_rf__wdata_r : da_rf__rdata [1];
      
      //
      adder__a  = da_src0 & {W{~da_ucode.src0_is_zero}};

      //
      casez ({da_ucode.has_special, da_ucode.has_imm})
        // Presently, only the CONTEXT.N special register has been
        // implemented.
        2'b1?:   adder__b_pre  = w_t'(bank_state_r [sort_bank_idx_r].n);
        2'b01:   adder__b_pre  = w_t'(da_ucode.imm);
        default: adder__b_pre  = da_src1;
      endcase // casez ({ucode.has_special, ucode.has_imm})

      //
      adder__b    = adder__b_pre ^ {W{da_ucode.inv_src1}};
      adder__cin  = da_ucode.cin;

    end // block: exe_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : fa_pc_PROC
      
      //
      fa_pc_en    = (fa_adv | fa_kill);

      //
      case (1'b1)
        da_taken_ret:     fa_pc_w  = pc_t'(da_src1);
        da_taken_call,
        da_taken_branch:  fa_pc_w  = da_ucode.target;
        default:          fa_pc_w  = fa_pc_r + 'b1;
      endcase // case (1'b1)

    end // block: fa_pc_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : da_rf_PROC

      //
      da_rf__wen_w  = da_adv & da_ucode.dst_en;
      da_rf__wa_w   = da_ucode.dst;
      priority casez ({da_ucode.is_pop, da_ucode.dst_is_blink, da_ucode.is_load})
        3'b1??:  da_rf__wdata_w  = stack__cmd_pop_dat_r;
        3'b010:  da_rf__wdata_w  = w_t'(fa_pc_r);
        3'b001:  da_rf__wdata_w  = sort__dout;
        default: da_rf__wdata_w  = adder__y;
      endcase // priority casez ({ucode.is_pop, ucode.dst_is_blink})

    end // block: da_rf_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : ar_flags_PROC

      //
      ar_flag_en       = da_adv & da_ucode.flag_en;
      ar_flag_c_w      = adder__cout;
      ar_flag_n_w      = adder__y [W - 1];
      ar_flag_z_w      = (adder__y == '0);

    end // block: da_flags_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : stack_PROC

      //
      stack__cmd_vld       = da_adv & (da_ucode.is_push | da_ucode.is_pop);
      stack__cmd_push      = da_ucode.is_push;
      stack__cmd_push_dat  = da_src1;
      stack__cmd_clr       = '0;

    end // block: stack_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : sort_PROC
      
      //
      sort__en          = da_mem_op_issue;
      sort__wen         = da_ucode.is_store;
      sort__addr        = addr_t'(da_ucode.is_store ? da_src0 : da_src1);
      sort__din         = da_src1;

      //
      sort__dout        = spsram_bank__dout [sort_bank_idx_r];

      // The 'momento' in this version is essentially just the rdata valid
      // as it is unnecessary to explicitly retain any state about the
      // operation.
      //
      sort_momento_in   = sort__en & (~sort__wen);

      //
      sort_bank_en      = da_adv & da_ucode.is_emit;
      sort_bank         = bank_state_r [sort_bank_idx_r];
      sort_bank.status  = BANK_SORTED;
      sort_bank.error   = '0;

      //
      sort_bank_idx_en  = sort_bank_en;
      sort_bank_idx_w   = sort_bank_idx_r + 'b1;

    end // block: sort_PROC
  
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
          bank_state_w [i]   |= (enqueue_bank_en ? enqueue_bank : '0);
        end
        if (bank_n_t'(i)   == sort_bank_idx_r) begin
          bank_state_en [i]  |= sort_bank_en;
          bank_state_w [i]   |= (sort_bank_en ? sort_bank : '0);
        end
        if (bank_n_t'(i)   == dequeue_bank_idx_r) begin
          bank_state_en [i]  |= dequeue_bank_en;
          bank_state_w [i]   |= (dequeue_bank_en ? dequeue_bank : '0);
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
      da_ld_stall_r <= 'b0;
    else
      da_ld_stall_r <= da_ld_stall_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      {ar_flag_z_r, ar_flag_n_r, ar_flag_c_r} <= 'b0;
    else if (ar_flag_en)
      {ar_flag_z_r, ar_flag_n_r, ar_flag_c_r} <=
          {ar_flag_z_w, ar_flag_n_w, ar_flag_c_w};
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      da_rf__wen_r <= '0;
    else
      da_rf__wen_r <= da_rf__wen_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (da_rf__wen_w) begin
      da_rf__wa_r    <= da_rf__wa_w;
      da_rf__wdata_r <= da_rf__wdata_w;
    end
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      fa_pc_r <= SYM_RESET;
    else if (fa_pc_en)
      fa_pc_r <= fa_pc_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      da_valid_r <= 'b0;
    else
      da_valid_r <= da_valid_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (da_en) begin
      da_inst_r <= da_inst_w;
      da_pc_r   <= da_pc_w;
    end

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
  fast_adder #(.W(W)) u_adder (
    //
      .y                 (adder__y           )
    , .cout              (adder__cout        )
    //
    , .a                 (adder__a           )
    , .b                 (adder__b           )
    , .cin               (adder__cin         )
  );
  
  // ------------------------------------------------------------------------ //
  //
  rf #(.W(W), .N(8), .RD_N(2)) u_rf (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .ra                (da_rf__ra          )
    , .ren               (da_rf__ren         )
    , .rdata             (da_rf__rdata       )
    //
    , .wa                (da_rf__wa_r        )
    , .wen               (da_rf__wen_r       )
    , .wdata             (da_rf__wdata_r     )
  );

  // ------------------------------------------------------------------------ //
  //
  stack #(.W(W), .N(128)) u_stack (
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
  delay_pipe #(.W(1), .N(1)) u_sort_momento_delay_pipe (
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
