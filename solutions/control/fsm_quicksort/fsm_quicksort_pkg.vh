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

package fsm_quicksort_pkg;

  localparam int N = 16;

  localparam int W  = 32;

  localparam int BANK_N  = 2;

  typedef logic [BANK_N-1:0]         bank_n_d_t;
  typedef logic [$clog2(BANK_N)-1:0] bank_n_t;
  typedef logic signed [$clog2(N):0] n_t;
  typedef logic [$clog2(N)-1:0]      addr_t;
  typedef logic [W-1:0]              w_t;

  typedef enum logic [2:0] {  BANK_IDLE       = 3'b000,
                              BANK_LOADING    = 3'b001,
                              BANK_READY      = 3'b010,
                              BANK_SORTING    = 3'b011,
                              BANK_SORTED     = 3'b100,
                              BANK_UNLOADING  = 3'b101
                            } bank_status_t;

  typedef struct packed {
    bank_status_t status;
    n_t          n;
    logic        error;
  } bank_state_t;
  localparam int BANK_CONTEXT_W  = $bits(bank_status_t);

  typedef enum   logic [2:0] {  ENQUEUE_FSM_IDLE  = 3'b000,
                                ENQUEUE_FSM_LOAD  = 3'b101
                                } enqueue_fsm_t;
  localparam int ENQUEUE_FSM_BUSY_B = 2;

  typedef enum   logic [2:0] {  DEQUEUE_FSM_IDLE  = 3'b000,
                                DEQUEUE_FSM_EMIT  = 3'b101
                                } dequeue_fsm_t;
  localparam int DEQUEUE_FSM_BUSY_B  = 2;

  typedef enum   logic [6:0] { SORT_FSM_IDLE          = 7'b0_000000,
                               SORT_FSM_START         = 7'b1_000001,
                               SORT_FSM_PARTITION_0   = 7'b1_010000,
                               SORT_FSM_PARTITION_1   = 7'b1_010001,
                               SORT_FSM_PARTITION_2   = 7'b1_010010,
                               SORT_FSM_PARTITION_3   = 7'b1_010011,
                               SORT_FSM_PARTITION_4   = 7'b1_010100,
                               SORT_FSM_PARTITION_5   = 7'b1_010101,
                               SORT_FSM_PARTITION_6   = 7'b1_010110,
                               SORT_FSM_PARTITION_7   = 7'b1_010111,
                               SORT_FSM_PARTITION_8   = 7'b1_011000,
                               SORT_FSM_PARTITION_9   = 7'b1_011001,
                               SORT_FSM_PARTITION_10  = 7'b1_011010,
                               SORT_FSM_PARTITION_11  = 7'b1_011011,
                               SORT_FSM_PARTITION_12  = 7'b1_011100,
                               SORT_FSM_PARTITION_13  = 7'b1_011101,
                               SORT_FSM_PARTITION_14  = 7'b1_011110,
                               SORT_FSM_PARTITION_15  = 7'b1_011111,
                               SORT_FSM_PARTITION_16  = 7'b1_100000,
                               SORT_FSM_QUICKSORT_LO  = 7'b1_000100,
                               SORT_FSM_QUICKSORT_HI  = 7'b1_000101,
                               SORT_FSM_DONE          = 7'b1_000110
                               } sort_fsm_t;
  localparam int SORT_FSM_BUSY_B = 6;

  typedef struct packed {
    sort_fsm_t      state;
    //
    n_t             lo;
    n_t             hi;
    //
    n_t             i;
    n_t             j;
    //
    w_t             dat_i;
    w_t             dat_j;
    w_t             dat_p;
    //
    logic           error;
  } sort_context_t;
  localparam int SORT_CONTEXT_W  = $bits(sort_context_t);

  typedef struct packed {
    logic        vld;
    w_t          dat;
  } reg_t;

endpackage // fsm_quicksort_pkg
  
