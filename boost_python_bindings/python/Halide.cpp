#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/str.hpp>
#include <boost/python.hpp>

//#include <Halide.h>
//#include "../../build/include/Halide.h"
#include "../../src/Var.h"
#include "../../src/Expr.h"
#include "../../src/IROperator.h"

#include "../../src/Func.h"

#include <string>

namespace h = Halide;
namespace p = boost::python;
using p::self;

char const* greet()
{
    return "hello, world from Halide python bindings";
}

/*
input = ImageParam(UInt(16), 2, 'input')
        x, y = Var('x'), Var('y')

blur_x = Func('blur_x')
        blur_y = Func('blur_y')

        blur_x[x,y] = (input[x,y]+input[x+1,y]+input[x+2,y])/3
        blur_y[x,y] = (blur_x[x,y]+blur_x[x,y+1]+blur_x[x,y+2])/3

        xi, yi = Var('xi'), Var('yi')

        blur_y.tile(x, y, xi, yi, 8, 4).parallel(y).vectorize(xi, 8)
        blur_x.compute_at(blur_y, x).vectorize(x, 8)

        maxval = 255
        in_image = Image(UInt(16), builtin_image('rgb.png'), scale=1.0) # Set scale to 1 so that we only use 0...255 of the UInt(16) range
        eval_func = filter_image(input, blur_y, in_image, disp_time=True, out_dims = (OUT_DIMS[0]-8, OUT_DIMS[1]-8), times=5)
        I = eval_func()
        if len(sys.argv) >= 2:
        I.save(sys.argv[1], maxval)
        else:
        I.show(maxval)
*/

template<typename PythonClass>
void add_operators(PythonClass &class_instance)
{
    using namespace boost::python;

    // FIXME Var + int, Var + float not yet working
    class_instance
            .def(self + self)
            .def(self - self)
            .def(self * self)
            .def(self / self)
            .def(self % self)
            //.def(pow(self, p::other<float>))
            .def(pow(self, self))
            .def(self & self) // and
            .def(self | self) // or
            .def(-self) // neg
            .def(~self) // invert
            .def(self < self)
            .def(self <= self)
            .def(self == self)
            .def(self != self)
            .def(self > self)
            .def(self >= self);

    return;
}

void defineVar()
{
    using Halide::Var;
    auto var_class = p::class_<Var>("Var",
                                    "A Halide variable, to be used when defining functions. It is just" \
                                    "a name, and can be reused in places where no name conflict will" \
                                    "occur. It can be used in the left-hand-side of a function" \
                                    "definition, or as an Expr. As an Expr, it always has type Int(32).\n" \
                                    "\n" \
                                    "Constructors::\n" \
                                    "Var()      -- Construct Var with an automatically-generated unique name\n" \
                                    "Var(name)  -- Construct Var with the given string name.\n",
                                    p::init<std::string>())
            .def(p::init<>())
            //.add_property("name", &Var::name) // "Get the name of a Var.")
            .def("name", &Var::name, boost::python::return_value_policy<boost::python:: copy_const_reference>())
            .def("same_as", &Var::same_as, "Test if two Vars are the same.")
            //.def(self == p::other<Var>())
            .def("implicit", &Var::implicit, "Construct implicit Var from int n.");

    add_operators(var_class);
    return;
}


void defineExpr()
{
    using Halide::Expr;

    auto expr_class = p::class_<Expr>("Expr",
                                      "An expression or fragment of Halide code.\n" \
                                      "One can explicitly coerce most types to Expr via the Expr(x) constructor." \
                                      "The following operators are implemented over Expr, and also other types" \
                                      "such as Image, Func, Var, RVar generally coerce to Expr when used in arithmetic::\n\n" \
                                      "+ - * / % ** & |\n" \
                                      "-(unary) ~(unary)\n" \
                                      " < <= == != > >=\n" \
                                      "+= -= *= /=\n" \
                                      "The following math global functions are also available::\n" \
                                      "Unary:\n" \
                                      "  abs acos acosh asin asinh atan atanh ceil cos cosh exp\n" \
                                      "  fast_exp fast_log floor log round sin sinh sqrt tan tanh\n" \
                                      "Binary:\n" \
                                      "  hypot fast_pow max min pow\n\n" \
                                      "Ternary:\n" \
                                      "  clamp(x, lo, hi)                  -- Clamp expression to [lo, hi]\n" \
                                      "  select(cond, if_true, if_false)   -- Return if_true if cond else if_false\n",
                                      p::init<std::string>())
            .def(p::init<int>()) // Make an expression representing a const 32-bit int (i.e. an IntImm)
            .def(p::init<float>()) // Make an expression representing a const 32-bit float (i.e. a FloatImm)
            .def(p::init<double>()) /* Make an expression representing a const 32-bit float, given a
                                                                             * double. Also emits a warning due to truncation. */
            .def(p::init<std::string>()) // Make an expression representing a const string (i.e. a StringImm)
            .def(p::init<const h::Internal::BaseExprNode *>()) //Expr(const Internal::BaseExprNode *n) : IRHandle(n) {}
            .def("type", &Expr::type); // Get the type of this expression node

    add_operators(expr_class);

    return;
}


h::Realization func_realize0(h::Func &that, std::vector<int32_t> sizes, const h::Target &target = h::Target())
{
    return that.realize(sizes, target);
}

BOOST_PYTHON_FUNCTION_OVERLOADS( func_realize0_overloads, func_realize0, 2, 3)


h::Realization func_realize1(h::Func &that, int x_size=0, int y_size=0, int z_size=0, int w_size=0,
                             const h::Target &target = h::Target())
{
    return that.realize(x_size, y_size, z_size, w_size, target);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(func_realize1_overloads, func_realize1, 1, 6)


void defineFunc()
{
    using Halide::Func;

    auto func_class = p::class_<Func>("Func",
                                      "A halide function. This class represents one stage in a Halide" \
                                      "pipeline, and is the unit by which we schedule things. By default" \
                                      "they are aggressively inlined, so you are encouraged to make lots" \
                                      "of little functions, rather than storing things in Exprs.\n" \
                                      "Constructors::\n\n" \
                                      "  Func()      -- Declare a new undefined function with an automatically-generated unique name\n" \
                                      "  Func(expr)  -- Declare a new function with an automatically-generated unique\n" \
                                      "                 name, and define it to return the given expression (which may\n" \
                                      "                 not contain free variables).\n" \
                                      "  Func(name)  -- Declare a new undefined function with the given name",
                                      p::init<>())
            .def(p::init<std::string>())
            .def(p::init<h::Expr>());
    //.def("set", &Func::set, "Typically one uses f[x, y] = expr to assign to a function. However f.set(expr) can be used also.")
/*
    func_class.def("allow_race_conditions",
                   &Func::allow_race_conditions,
                   "Specify that race conditions are permitted for this Func, "
                   "which enables parallelizing over RVars even when Halide cannot "
                   "prove that it is safe to do so. Use this with great caution, "
                   "and only if you can prove to yourself that this is safe, as it "
                   "may result in a non-deterministic routine that returns "
                   "different values at different times or on different machines.");*/

    func_class.def("realize",
                   &func_realize1,
                   func_realize1_overloads(
                       p::args("x_size", "y_size", "z_size", "w_size", "target"),
                       "Evaluate this function over some rectangular domain and return"
                       "the resulting buffer. The buffer should probably be instantly"
                       "wrapped in an Image class.\n\n" \
                       "One can use f.realize(Buffer) to realize into an existing buffer."))
            .def("realize", &func_realize0, func_realize0_overloads(
                     p::args("sizes", "target")));


    /*
                            &Func::compile_to_bitcode(self, filename, list_of_Argument, fn_name=""):
                            """
                            Statically compile this function to llvm bitcode, with the
                            given filename (which should probably end in .bc), type
                            signature, and C function name (which defaults to the same name
                                                            as this halide function.
                                                            """

                                                            &Func::compile_to_c(self, filename, list_of_Argument, fn_name=""):
                                                            """
                                                            Statically compile this function to C source code. This is
                                                            useful for providing fallback code paths that will compile on
                                                            many platforms. Vectorization will fail, and parallelization
                                                            will produce serial code.
                                                            """

                                                            &Func::compile_to_file(self, filename_prefix, list_of_Argument, target):
                                                            """
                                                            Various signatures::

                                                            compile_to_file(filename_prefix, list_of_Argument)
                                                            compile_to_file(filename_prefix)
                                                            compile_to_file(filename_prefix, Argument a)
                                                            compile_to_file(filename_prefix, Argument a, Argument b)

                                                            Compile to object file and header pair, with the given
                                                            arguments. Also names the C function to match the first
                                                            argument.
                                                            """
*/

/*
                                                            &Func::compile_jit(self):
                                                            """
                                                            Eagerly jit compile the function to machine code. This
                                                            normally happens on the first call to realize. If you're
                                                            running your halide pipeline inside time-sensitive code and
                                                            wish to avoid including the time taken to compile a pipeline,
                                                            then you can call this ahead of time. Returns the raw function
                                                            pointer to the compiled pipeline.
                                                            """

                                                            &Func::debug_to_file(self, filename):
                                                            """
                                                            When this function is compiled, include code that dumps its values
                                                            to a file after it is realized, for the purpose of debugging.
                                                            The file covers the realized extent at the point in the schedule that
                                                            debug_to_file appears.

                                                            If filename ends in ".tif" or ".tiff" (case insensitive) the file
                                                            is in TIFF format and can be read by standard tools.
                                                            """

                                                            &Func::name(self):
                                                            """
                                                            The name of this function, either given during construction, or automatically generated.
                                                            """

                                                            &Func::value(self):
                                                            """
                                                            The right-hand-side value of the pure definition of this
                                                            function. May be undefined if the function has no pure
                                                            definition yet.
                                                            """

                                                            &Func::dimensions(self):
                                                            """
                                                            The dimensionality (number of arguments) of this
                                                            function. Zero if the function is not yet defined.
                                                            """
*/
                                                          /*
                                                            &Func::__getitem__(self, *args):
                                                            """
                                                            Either calls to the function, or the left-hand-side of a
                                                            reduction definition (see \ref RDom). If the function has
                                                            already been defined, and fewer arguments are given than the
                                                            function has dimensions, then enough implicit vars are added to
                                                            the end of the argument list to make up the difference.
                                                            """

                                                            &Func::split(self, old, outer, inner, factor):
                                                            """
                                                            Split a dimension into inner and outer subdimensions with the
                                                            given names, where the inner dimension iterates from 0 to
                                                            factor-1. The inner and outer subdimensions can then be dealt
                                                            with using the other scheduling calls. It's ok to reuse the old
                                                            variable name as either the inner or outer variable.

                                                            The arguments are all Var instances.
                                                            """

                                                            &Func::fuse(self, inner, outer, fused):
                                                            """
                                                            Join two dimensions into a single fused dimension. The fused
                                                            dimension covers the product of the extents of the inner and
                                                            outer dimensions given.
                                                            """

                                                            &Func::parallel(self, var):
                                                            """
                                                            Mark a dimension (Var instance) to be traversed in parallel.
                                                            """

                                                            &Func::vectorize(self, var, factor):
                                                            """
                                                            Split a dimension (Var instance) by the given int factor, then vectorize the
                                                            inner dimension. This is how you vectorize a loop of unknown
                                                            size. The variable to be vectorized should be the innermost
                                                            one. After this call, var refers to the outer dimension of the
                                                            split.
                                                            """

                                                            &Func::unroll(self, var, factor=None):
                                                            """
                                                            Split a dimension (Var instance) by the given int factor, then unroll the inner
                                                            dimension. This is how you unroll a loop of unknown size by
                                                            some constant factor. After this call, var refers to the outer
                                                            dimension of the split.
                                                            """

                                                            &Func::bound(self, min_expr, extent_expr):
                                                            """
                                                            Statically declare that the range over which a function should
                                                            be evaluated is given by the second and third arguments. This
                                                            can let Halide perform some optimizations. E.g. if you know
                                                            there are going to be 4 color channels, you can completely
                                                            vectorize the color channel dimension without the overhead of
                                                            splitting it up. If bounds inference decides that it requires
                                                            more of this function than the bounds you have stated, a
                                                            runtime error will occur when you try to run your pipeline.
                                                            """

                                                            &Func::tile(self, x, y, xo, yo, xi, yi, xfactor, yfactor):
                                                            """
                                                            Traverse in tiled order. Two signatures::

                                                            tile(x, y, xi, yi, xfactor, yfactor)
                                                            tile(x, y, xo, yo, xi, yi, xfactor, yfactor)

                                                            Split two dimensions at once by the given factors, and then
                                                            reorder the resulting dimensions to be xi, yi, xo, yo from
                                                            innermost outwards. This gives a tiled traversal.

                                                            The shorter form of tile reuses the old variable names as
                                                            the new outer dimensions.
                                                            """

                                                            &Func::reorder(self, *args):
                                                            """
                                                            Reorder the dimensions (Var arguments) to have the given nesting
                                                            order, from innermost loop order to outermost.
                                                            """

                                                            &Func::rename(self, old_name, new_name):
                                                            """
                                                            Rename a dimension. Equivalent to split with a inner size of one.
                                                            """

                                                            &Func::gpu_threads(self, *args):
                                                            """
                                                            Tell Halide that the following dimensions correspond to cuda
                                                            thread indices. This is useful if you compute a producer
                                                            function within the block indices of a consumer function, and
                                                            want to control how that function's dimensions map to cuda
                                                            threads. If the selected target is not ptx, this just marks
                                                            those dimensions as parallel.
                                                            """

                                                            &Func::gpu_single_thread(self, *args):
                                                            """
                                                            Tell Halide to run this stage using a single gpu thread and
                                                            block. This is not an efficient use of your GPU, but it can be
                                                            useful to avoid copy-back for intermediate update stages that
                                                            touch a very small part of your Func.
                                                            """

                                                            &Func::gpu_blocks(self, *args):
                                                            """
                                                            Tell Halide that the following dimensions correspond to cuda
                                                            block indices. This is useful for scheduling stages that will
                                                            run serially within each cuda block. If the selected target is
                                                            not ptx, this just marks those dimensions as parallel.
                                                            """

                                                            &Func::gpu(self, block_x, thread_x):
                                                            """
                                                            Three signatures::

                                                            gpu(block_x, thread_x)
                                                            gpu(block_x, block_y, thread_x, thread_y)
                                                            gpu(block_x, block_y, block_z, thread_x, thread_y, thread_z)

                                                            Tell Halide that the following dimensions correspond to cuda
                                                            block indices and thread indices. If the selected target is not
                                                            ptx, these just mark the given dimensions as parallel. The
                                                            dimensions are consumed by this call, so do all other
                                                            unrolling, reordering, etc first.
                                                            """

                                                            &Func::gpu_tile(self, x, x_size):
                                                            """
                                                            Three signatures:

                                                            gpu_tile(x, x_size)
                                                            gpu_tile(x, y, x_size, y_size)
                                                            gpu_tile(x, y, z, x_size, y_size, z_size)

                                                            Short-hand for tiling a domain and mapping the tile indices
                                                            to cuda block indices and the coordinates within each tile to
                                                            cuda thread indices. Consumes the variables given, so do all
                                                            other scheduling first.
                                                            """

                                                            &Func::cuda_threads(self, *args):
                                                            """
                                                            deprecated Old name for #gpu_threads.
                                                            Tell Halide that the following dimensions correspond to cuda
                                                            thread indices. This is useful if you compute a producer
                                                            function within the block indices of a consumer function, and
                                                            want to control how that function's dimensions map to cuda
                                                            threads. If the selected target is not ptx, this just marks
                                                            those dimensions as parallel.
                                                            """

                                                            &Func::cuda_blocks(self, *args):
                                                            """
                                                            deprecated Old name for #cuda_blocks.
                                                            Tell Halide that the following dimensions correspond to cuda
                                                            block indices. This is useful for scheduling stages that will
                                                            run serially within each cuda block. If the selected target is
                                                            not ptx, this just marks those dimensions as parallel.
                                                            """

                                                            &Func::cuda(self, block_x, thread_x):
                                                            """
                                                            deprecated Old name for #cuda.
                                                            Three signatures::

                                                            cuda(block_x, thread_x)
                                                            cuda(block_x, block_y, thread_x, thread_y)
                                                            cuda(block_x, block_y, block_z, thread_x, thread_y, thread_z)

                                                            Tell Halide that the following dimensions correspond to cuda
                                                            block indices and thread indices. If the selected target is not
                                                            ptx, these just mark the given dimensions as parallel. The
                                                            dimensions are consumed by this call, so do all other
                                                            unrolling, reordering, etc first.
                                                            """

                                                            &Func::cuda_tile(self, x, x_size):
                                                            """
                                                            deprecated Old name for #cuda_tile.
                                                            Three signatures:

                                                            cuda_tile(x, x_size)
                                                            cuda_tile(x, y, x_size, y_size)
                                                            cuda_tile(x, y, z, x_size, y_size, z_size)

                                                            Short-hand for tiling a domain and mapping the tile indices
                                                            to cuda block indices and the coordinates within each tile to
                                                            cuda thread indices. Consumes the variables given, so do all
                                                            other scheduling first.
                                                            """

                                                            &Func::reorder_storage(self, *args):
                                                            """
                                                            Scheduling calls that control how the storage for the function
                                                            is laid out. Right now you can only reorder the dimensions.
                                                            """

                                                            &Func::compute_at(self, f, var):
                                                            """
                                                            Compute this function as needed for each unique value of the
                                                            given var (can be a Var or an RVar) for the given calling function f.
                                                            """

                                                            &Func::compute_root(self):
                                                            """
                                                            Compute all of this function once ahead of time.
                                                            """

                                                            &Func::store_at(self, f, var):
                                                            """
                                                            Allocate storage for this function within f's loop over
                                                            var (can be a Var or an RVar). Scheduling storage is optional, and can be used to
                                                            separate the loop level at which storage occurs from the loop
                                                            level at which computation occurs to trade off between locality
                                                            and redundant work.
                                                            """

                                                            &Func::store_root(self):
                                                            """
                                                            Equivalent to Func.store_at, but schedules storage outside the outermost loop.
                                                            """

                                                            &Func::compute_inline(self):
                                                            """
                                                            Aggressively inline all uses of this function. This is the
                                                            default schedule, so you're unlikely to need to call this. For
                                                            a reduction, that means it gets computed as close to the
                                                            innermost loop as possible.
                                                            """

                                                            &Func::update(self):
                                                            """
                                                            Get a handle on the update step of a reduction for the
                                                            purposes of scheduling it. Only the pure dimensions of the
                                                            update step can be meaningfully manipulated (see RDom).
                                                            """

                                                            &Func::function(self):
                                                            """
                                                            Get a handle on the internal halide function that this Func
                                                            represents. Useful if you want to do introspection on Halide
                                                            functions.
                                                            """
                */
    ;


    return;
}

BOOST_PYTHON_MODULE(halide)
{
    using namespace boost::python;
    def("greet", greet);

    defineVar();
    defineExpr();
    defineFunc();
}
