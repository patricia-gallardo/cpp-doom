# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

function(set_project_warnings)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" FALSE)

    set(MSVC_WARNINGS
#            /W4 # Baseline reasonable warnings
#            /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss of data
#            /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
#            /w14263 # 'function': member function does not override any base class virtual member function
#            /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
#            # be destructed correctly
#            /w14287 # 'operator': unsigned/negative constant mismatch
#            /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
#            # the for-loop scope
#            /w14296 # 'operator': expression is always 'boolean_value'
#            /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
#            /w14545 # expression before comma evaluates to a function which is missing an argument list
#            /w14546 # function call before comma missing argument list
#            /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
#            /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
#            /w14555 # expression has no effect; expected expression with side- effect
#            /w14619 # pragma warning: there is no warning number 'number'
#            /w14640 # Enable warning on thread un-safe static member initialization
#            /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
#            /w14905 # wide string literal cast to 'LPSTR'
#            /w14906 # string literal cast to 'LPWSTR'
#            /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
#            /permissive- # standards conformance mode for MSVC compiler.
            )

    set(CLANG_WARNINGS
            -Wall
            -Wextra # reasonable and standard
            -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
            # catch hard to track down memory errors
            # -Wold-style-cast # warn for c-style casts TODO turn it back on (patricia)
            # -Wcast-align # warn for potential performance problem casts TODO turn it back on (patricia)
            # -Wunused # warn on anything being unused TODO turn it back on (patricia)
            -Woverloaded-virtual # warn if you overload (not override) a virtual function
            # -Wpedantic # warn if non-standard C++ is used TODO turn it back on (patricia)
            # -Wconversion # warn on type conversions that may lose data TODO turn it back on (patricia)
            -Wno-sign-conversion # warn on sign conversions TODO turn it back on (patricia)
            -Wnull-dereference # warn if a null dereference is detected
            -Wdouble-promotion # warn if float is implicit promoted to double
            -Wformat=2 # warn on security issues around functions that format output (ie printf)
            #-Wno-unused-lambda-capture # CLANG We like explicit capture
            -Wno-unused-parameter
            #-Wno-unused-variable # TODO turn it back on (patricia)
            -Wno-unused-function # TODO turn it back on (patricia)
            #-Wno-reserved-user-defined-literal # CLANG TODO turn it back on (patricia)?
            #-Wno-shorten-64-to-32 # CLANG TODO turn it back on (patricia)
            #-Wno-missing-field-initializers # TODO turn it back on (patricia)
            #-Wno-implicit-int-conversion # CLANG TODO turn it back on (patricia)
            -Wno-sign-compare # TODO turn it back on (patricia)
            #-Wno-writable-strings # CLANG TODO turn it back on (patricia)
            -Wno-format-nonliteral # TODO turn it back on (patricia)
            #-Wno-macro-redefined # CLANG TODO turn it back on (patricia)
            -Wno-float-conversion # TODO turn it back on (patricia)
            -Wno-double-promotion # TODO turn it back on (patricia)
            #-Wno-implicit-float-conversion # CLANG TODO turn it back on (patricia)
            #-Wno-c++11-narrowing # CLANG TODO turn it back on (patricia)
            #-Wno-deprecated-anon-enum-enum-conversion # CLANG TODO turn it back on (patricia)
            -Wno-narrowing # TODO turn it back on (patricia)
            -Wno-unknown-warning-option # CLANG TODO turn it back on (patricia)
            -fpermissive # GCC TODO turn it back on (patricia)
            -Wno-unused-but-set-parameter # GCC TODO turn it back on (patricia)
            -Wno-parentheses # GCC TODO turn it back on (patricia)
            -Wno-int-in-bool-context # GCC TODO turn it back on (patricia)
            -Wno-class-memaccess # GCC TODO turn it back on (patricia)
            )

    if (WARNINGS_AS_ERRORS)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
    endif ()

    if (WIN32)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    else ()
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    endif ()

    add_compile_options(${PROJECT_WARNINGS})

endfunction()
