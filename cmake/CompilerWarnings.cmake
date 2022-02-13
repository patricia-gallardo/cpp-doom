# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

function(set_project_warnings)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" TRUE)

    set(MSVC_WARNINGS
            /W4 # Baseline reasonable warnings
#            /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss of data
            /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
            /w14263 # 'function': member function does not override any base class virtual member function
            /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
            # be destructed correctly
#            /w14287 # 'operator': unsigned/negative constant mismatch
            /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
            # the for-loop scope
            /w14296 # 'operator': expression is always 'boolean_value'
            /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
            /w14545 # expression before comma evaluates to a function which is missing an argument list
            /w14546 # function call before comma missing argument list
            /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
            /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
            /w14555 # expression has no effect; expected expression with side- effect
            /w14619 # pragma warning: there is no warning number 'number'
            /w14640 # Enable warning on thread un-safe static member initialization
            /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
            /w14905 # wide string literal cast to 'LPSTR'
            /w14906 # string literal cast to 'LPWSTR'
            /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
            /permissive- # standards conformance mode for MSVC compiler.
#            /wd4018 # disable warning C4018: '<': signed/unsigned mismatch TODO turn it back on (patricia)
#            /wd4389 # disable warning C4389: '==': signed/unsigned mismatch TODO turn it back on (patricia)
#            /wd4244 # disable warning C4244: '=': conversion, possible loss of data TODO turn it back on (patricia)
#            /wd4267 # disable warning C4267: '=': conversion, possible loss of data TODO turn it back on (patricia)
            /wd4996 # disable warning C4996: was declared deprecated TODO turn it back on (patricia)
            /wd4068 # disable warning C4068: unknown pragma warnings
            /wd5222 # disable warning C5222: 'unreachable': all unscoped attribute names are reserved for future standardization
            /wd5030 # disable warning C5030: attribute 'unreachable' is not recognized
            /wd4706 # disable warning C4706: assignment within conditional expression
            )

    set(CLANG_WARNINGS
            -Wall
            -Wextra # reasonable and standard
            -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
            # catch hard to track down memory errors
            -Wold-style-cast # warn for c-style casts TODO turn it back on (patricia)
            # -Wcast-align # warn for potential performance problem casts TODO turn it back on (patricia)
            -Wunused # warn on anything being unused
            -Woverloaded-virtual # warn if you overload (not override) a virtual function
            -Wpedantic # warn if non-standard C++ is used TODO turn it back on (patricia)
            -Wconversion # warn on type conversions that may lose data TODO turn it back on (patricia)
            -Wnull-dereference # warn if a null dereference is detected
            -Wdouble-promotion # warn if float is implicit promoted to double
            #-Wformat=2 # warn on security issues around functions that format output (ie printf) TODO turn it back on (patricia)
            -Wno-format-pedantic # CLANG TODO turn it back on (patricia)
            -Wno-format # GCC TODO turn it back on (patricia)
            -Wno-format-nonliteral # TODO turn it back on (patricia)
            -Wno-sign-conversion # warn on sign conversions TODO turn it back on (patricia)
            #-Wno-sign-compare # TODO turn it back on (patricia)
            -Wno-float-conversion # TODO turn it back on (patricia)
            -Wno-double-promotion # TODO turn it back on (patricia)
            -Wno-narrowing # TODO turn it back on (patricia)
            -Wno-unknown-warning-option # CLANG TODO turn it back on (patricia)
            -Wno-class-memaccess # GCC TODO turn it back on (patricia)
            -Wno-unknown-pragmas # GCC TODO turn it back on (patricia)
            -Wno-unknown-attributes # unknown attribute 'unreachable' ignored
            -Wno-attributes
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
