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

`ifndef __VERT_UCODE_QUICKSORT_PKG_VH__
`define __VERT_UCODE_QUICKSORT_PKG_VH__

package vert_ucode_quicksort_pkg;

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

  typedef logic [7:0] pc_t;

  typedef struct packed {
    logic        foo;
  } sort_context_t;
  localparam int SORT_CONTEXT_W  = $bits(sort_context_t);

  typedef enum   logic [1:0] { UNCOND  = 2'b00,
                               EQ      = 2'b01,
                               GT      = 2'b10,
                               LE      = 2'b11
                               } cc_t;

  typedef logic [2:0] imm_t;
  typedef logic [7:0] field_A_t;

  typedef enum        logic [2:0] { R0     = 3'b000,
                                    R1     = 3'b001,
                                    R2     = 3'b010,
                                    R3     = 3'b011,
                                    R4     = 3'b100,
                                    R5     = 3'b101,
                                    R6     = 3'b110,
                                    BLINK  = 3'b111} reg_t;

  typedef enum        logic [2:0] { REG_N
                                    } reg_special_t;

  typedef enum logic [3:0] { NOP    = 4'b0000,
                             JCC    = 4'b0001,
                             PP     = 4'b0010,
                             MEM    = 4'b0100,
                             MOV    = 4'b0110,
                             ARITH  = 4'b0111,
                             CRET   = 4'b1100,
                             CNTRL  = 4'b1111
                            } opcode_t;

  typedef struct packed {
    opcode_t opcode;
    union packed {
      logic [11:0] raw;
      struct packed {
        logic [11:0] padding;
      } nop;
      struct packed {
        logic [1:0] padding;
        cc_t cc;
        field_A_t A;
      } jcc;
      struct  packed {
        logic is_pop;
        union packed {
          struct packed {
            reg_t dst;
            logic [7:0] padding0;
          } pop;
          struct packed {
            logic [7:0] padding0;
            reg_t src1;
          } push;
        } u;
      } pp;
      struct  packed {
        logic is_st;
        reg_t dst;
        union packed {
          struct packed {
            logic [3:0] padding0;
            logic       padding1;
            reg_t src1;
          } ld;
          struct packed {
            logic padding0;
            reg_t src0;
            logic padding1;
            reg_t src1;
          } st;
        } u;
      } mem;
      struct  packed {
        logic is_special;
        reg_t dst;
        logic [3:0] padding0;
        logic is_imm;
        union packed {
          reg_t src;
          reg_special_t special;
          imm_t imm;
        } u;
      } mov;
      struct  packed {
        logic is_sub;
        reg_t dst;
        logic wren;
        reg_t src0;
        logic is_imm;
        union packed {
          reg_t src1;
          imm_t imm;
        } u;
      } arith;
      struct  packed {
        logic is_ret;
        logic [2:0] padding;
        field_A_t a;
      } cret;
      struct  packed {
        logic is_emit;
        logic [10:0] padding;
      } cntrl;
    } u;
  } inst_t;


  typedef struct     packed {
    logic            is_emit;
    logic            is_wait;
    logic            is_call;
    logic            is_ret;
    logic            is_load;
    logic            is_store;
    logic            is_jump;
    logic            is_push;
    logic            is_pop;
    
    //
    logic            dst_en;
    reg_t            dst;

    //
    logic            dst_is_blink;

    //
    logic            src0_en;
    reg_t            src0;
    logic            src1_en;
    reg_t            src1;

    //
    logic            src0_is_zero;

    //
    logic            has_imm;
    imm_t            imm;

    //
    logic            has_special;
    reg_special_t    special;

    //
    logic            inv_src1;
    logic            cin;

    //
    logic            flag_en;

    //
    logic            invalid_inst;

    //
    cc_t             cc;
    field_A_t        target;
    
  } ucode_t;

  function logic [2:0] R_field(inst_t inst);
    return inst[10:8];
  endfunction // R_field

  function logic [2:0] S_field(inst_t inst);
    return inst[6:4];
  endfunction // S_field

  function logic [2:0] SPECIAL_field(inst_t inst);
    return inst[2:0];
  endfunction // SPECIAL_field

  function logic [2:0] U_field(inst_t inst);
    return inst[2:0];
  endfunction // U_field

  function logic [2:0] I_field(inst_t inst);
    return inst[2:0];
  endfunction // I_field

  function logic IMM_field(inst_t inst);
    return inst[3];
  endfunction // I_field

  function logic W_field(inst_t inst);
    return inst[7];
  endfunction // W_field

  function logic SEL_field(inst_t inst);
    return inst[11];
  endfunction // SEL_field

  function logic [1:0] CC_field(inst_t inst);
    return inst[9:8];
  endfunction // CC_field

  function logic [7:0] A_field(inst_t inst);
    return inst[7:0];
  endfunction // CC_field

  function ucode_t decode(inst_t inst); begin
    logic sel      = SEL_field(inst);
    ucode_t ucode  = '0;
    // COMMON
    ucode.imm      = I_field(inst);
    ucode.dst      = R_field(inst);
    ucode.src0     = S_field(inst);
    ucode.src1     = U_field(inst);
    ucode.special  = SPECIAL_field(inst);
    ucode.cc       = CC_field(inst);
    ucode.target   = A_field(inst);
    
    case (inst.opcode)
      NOP: begin
      end
      JCC: begin
        // Jcc
        ucode.is_jump  = 'b1;
      end
      PP: begin
        case (sel)
          1'b1: begin
            // POP
            ucode.is_pop  = 'b1;
            ucode.dst_en  = 'b1;
          end
          default: begin
            // PUSH
            ucode.is_push       = 'b1;
            ucode.src1_en       = 'b1;
            ucode.src0_is_zero  = 'b1;
          end
        endcase
      end
      MEM: begin
        ucode.src1_en   = 'b1;
        case (sel)
          1'b1: begin
            // ST
            ucode.is_store  = 'b1;
            ucode.src0_en   = 'b1;
          end
          default: begin
            // LD
            ucode.is_load  = 'b1;
            ucode.dst_en   = 'b1;
          end
        endcase // case (sel)
      end
      MOV: begin
        ucode.dst_en        = 'b1;
        ucode.src0_is_zero  = 'b1;
        unique0 casez ({inst[11],inst[3]})
          // MOV
          2'b00:   ucode.src1_en      = 'b1;
          // MOVI
          2'b01:   ucode.has_imm      = 'b1;
          // MOVS
          2'b1?:   ucode.has_special  = 'b1;
        endcase // unique0 casez ({inst[11],inst[3]})
      end
      ARITH: begin
        ucode.dst_en   = W_field(inst);
        ucode.flag_en  = 'b1;
        ucode.src0_en  = 'b1;
        if (IMM_field(inst))
          // {ADD,SUB}I
          ucode.has_imm  = 'b1;
        else
          // {ADD,SUB}
          ucode.src1_en  = 'b1;
        if (sel) begin
          // SUB
          ucode.inv_src1  = 'b1;
          ucode.cin       = 'b1;
        end
      end
      CRET: begin
        ucode.is_jump       = 'b1;
        ucode.src0_is_zero  = 'b1;
        case (sel)
          1'b1: begin
            // RET
            ucode.is_ret   = 'b1;
            ucode.src1_en  = 'b1;
            ucode.src1     = BLINK;
          end
          default: begin
            // CALL
            ucode.is_call  = 'b1;
            ucode.dst_en   = 'b1;
            ucode.dst      = BLINK;
          end
        endcase // case (inst.u.cret.is_call)
      end
      CNTRL: begin
        if (sel)
          ucode.is_emit  = 'b1;
        else
          ucode.is_wait  = 'b1;
      end
      default: begin
        ucode.invalid_inst  = 'b1;
      end
    endcase // case (inst.opcode)

    //
    ucode.dst_is_blink  = (ucode.dst == BLINK);
    
    return ucode;
  end endfunction
    

endpackage // fsm_quicksort_pkg
  
`endif //  `ifndef __VERT_UCODE_QUICKSORT_PKG_VH__
