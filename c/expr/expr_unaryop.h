//------------------------------------------------------------------------------
// Copyright 2018-2019 H2O.ai
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//------------------------------------------------------------------------------
#ifndef dt_EXPR_EXPR_UNARYOP_h
#define dt_EXPR_EXPR_UNARYOP_h
#include "expr/by_node.h"
#include "expr/expr.h"
#include "python/_all.h"
#include "python/args.h"
#include "utils/function.h"
namespace dt {
namespace expr {


/**
 * `base_expr` class that implements computation of unary operators
 * and single-argument functions.
 */
class expr_unaryop : public base_expr {
  private:
    pexpr arg;
    Op opcode;

  public:
    expr_unaryop(pexpr&& a, Op op);
    SType resolve(const EvalContext& ctx) override;
    GroupbyMode get_groupby_mode(const EvalContext&) const override;
    Column evaluate(EvalContext& ctx) override;

    bool is_negated_expr() const override;
    pexpr get_negated_expr() override;
};



/**
 * Python-facing function that implements an unary operator / single-
 * argument function. This function can take as an argument either a
 * python scalar, or an f-expression, or a Frame (in which case the
 * function is applied to all elements of the frame).
 */
py::oobj unary_pyfn(const py::PKArgs&);




//------------------------------------------------------------------------------
// unary_infos
//------------------------------------------------------------------------------

/**
 * Singleton class (the instance is `unary_library`) containing the
 * information about all unary functions, and how they should be
 * applied.
 *
 * Specifically, a unary function is selected via an opcode + the
 * argument's stype. For example, `abs(x)` is a unary function with
 * opcode `Op::ABS` and several overloads for inputs of types INT8,
 * INT16, INT32, INT64, FLOAT32, FLOAT64, etc.
 *
 * Public API
 * ----------
 *
 * - get_infon(opcode, input_stype) -> const uinfo*
 * - get_infox(opcode, input_stype) -> const uinfo&
 *
 *     For a given opcode+stype return the record describing this
 *     unary function. The two methods only differ in their behavior
 *     when such record is not found: `get_infon` returns a nullptr
 *     in this case, whereas `get_infox` throws an exception.
 *
 *     See below for the description of `uinfo` object returned from
 *     this function.
 *
 * - get_opcode_from_args(args) -> Op
 *
 *     If a function for a certain opcode was exported into python,
 *     then such function will have an associated `py::PKArgs`
 *     object. The method `get_opcode_from_args` retrieves the
 *     opcode corresponding to this PKArgs object.
 *
 *     This method is only used inside `unary_pyfn()`, allowing us
 *     to reuse the same function (albeit with different PKArgs)
 *     for all unary funcs exposed into python.
 *
 * uinfo
 * -----
 * Information about each valid unary function is stored in the
 * `uinfo` struct, which has the following fields:
 *
 * - scalarfn (erased_func_t)
 *
 *     Function that can be applied to a single (scalar) input. This
 *     function is type-erased than, but generally it has signature
 *     `TO(*)(TI)`. The caller is expected to know the actual
 *     signature in order to use this function.
 *
 * - vcolfn (vcol_func_t)
 *
 *     Function that can be used to produce a virtual column
 *     corresponding to this unary transform. This function takes a
 *     single argument: a virtual column that is the argument of the
 *     transform (possibly after applying cast specified in the
 *     `cast_type` field).
 *
 * - output_stype (SType)
 *
 *     The SType of the output column produced from this ufunc.
 *
 * - cast_stype (SType)
 *
 *     If this field is non-empty (empty being SType::VOID), then the
 *     input column must be cast into this stype before being passed
 *     to `vcolfn`.
 *
 */
class unary_infos {
  using vcol_func_t = Column(*)(Column&& arg);
  using erased_func_t = void(*)();

  public:
    struct uinfo {
      erased_func_t scalarfn;
      vcol_func_t   vcolfn;
      SType output_stype;
      SType cast_stype;
      size_t : 48;
    };

    unary_infos();
    const uinfo* get_infon(Op opcode, SType input_stype) const;
    const uinfo& get_infox(Op opcode, SType input_stype) const;
    Op get_opcode_from_args(const py::PKArgs&) const;

  private:
    std::unordered_map<size_t /* Op, SType */, uinfo> info;
    std::unordered_map<size_t /* Op */, std::string> names;
    std::unordered_map<const py::PKArgs*, Op> opcodes;

    static constexpr size_t id(Op) noexcept;
    static constexpr size_t id(Op, SType) noexcept;

    void add_op(Op op, const char* name, const py::PKArgs* args);
    void add_copy(Op op, SType input_stype, SType output_stype);
    template <Op OP, SType SI, SType SO, typename _readt<SO>::t(*)(typename _readt<SI>::t)>
    void add();
    template <Op OP, SType SI, SType SO, bool(*)(typename _readt<SI>::t, bool, typename _readt<SO>::t*)>
    void add();


    template <float(*F32)(float), double(*F64)(double)>
    void add_math(Op, const char*, const py::PKArgs&);
};


extern unary_infos unary_library;



}}
#endif
