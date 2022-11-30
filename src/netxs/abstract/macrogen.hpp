// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#if defined(MACROGEN_DEF)
    #undef MACROGEN_DEF

        #define CAT_(x, ...) x ## __VA_ARGS__
        #define CAT(x, ...) CAT_(x, __VA_ARGS__)

        #define EVAL_(...) __VA_ARGS__
        #define EVAL(...) EVAL_(__VA_ARGS__)

        #define WRAP__odd(...) ((__VA_ARGS__))WRAP_even
        #define WRAP_even(...) ((__VA_ARGS__))WRAP__odd
        #define WRAP_even_last
        #define WRAP__odd_last
        #define WRAP(args) EVAL(CAT(WRAP__odd args, _last))

        #define TAKE_NAME(...) __VA_OPT__(this->__VA_ARGS__) // Ignore trailing spaces.
        #define MAKE_NAME(type, name, ...) TAKE_NAME(name)
        #define MAKE_INIT(type, name, ...) this->name = name;
        #define MAKE_ATTR(type, name, ...) type name{};
        #define MAKE_SIGN(type, name, ...) type name
        #define MAKE_TYPE(type, name, ...) type
        #define MAKE_TEMP(type, name, ...) this->name = source. name;
        #define MAKE_LOGS(type, name, ...) s << "\n\t " << #name << ": " << o.name;
        #define MAKE_WIPE(type, name, ...) this->name = {};

        #define SEQ_ATTR__odd(...) MAKE_ATTR __VA_ARGS__ SEQ_ATTR_even
        #define SEQ_ATTR_even(...) MAKE_ATTR __VA_ARGS__ SEQ_ATTR__odd
        #define SEQ_ATTR_even_last
        #define SEQ_ATTR__odd_last
        #define SEQ_ATTR(args) EVAL(CAT(SEQ_ATTR__odd args, _last))

        #define SEQ_INIT__odd(...) MAKE_INIT __VA_ARGS__ SEQ_INIT_even
        #define SEQ_INIT_even(...) MAKE_INIT __VA_ARGS__ SEQ_INIT__odd
        #define SEQ_INIT_even_last
        #define SEQ_INIT__odd_last
        #define SEQ_INIT(args) EVAL(CAT(SEQ_INIT__odd args, _last))

        #define SEQ_TEMP__odd(...) MAKE_TEMP __VA_ARGS__ SEQ_TEMP_even
        #define SEQ_TEMP_even(...) MAKE_TEMP __VA_ARGS__ SEQ_TEMP__odd
        #define SEQ_TEMP_even_last
        #define SEQ_TEMP__odd_last
        #define SEQ_TEMP(args) EVAL(CAT(SEQ_TEMP__odd args, _last))

        #define SEQ_WIPE__odd(...) MAKE_WIPE __VA_ARGS__ SEQ_WIPE_even
        #define SEQ_WIPE_even(...) MAKE_WIPE __VA_ARGS__ SEQ_WIPE__odd
        #define SEQ_WIPE_even_last
        #define SEQ_WIPE__odd_last
        #define SEQ_WIPE(args) EVAL(CAT(SEQ_WIPE__odd args, _last))

        #define SEQ_LOGS__odd(...) MAKE_LOGS __VA_ARGS__ SEQ_LOGS_even
        #define SEQ_LOGS_even(...) MAKE_LOGS __VA_ARGS__ SEQ_LOGS__odd
        #define SEQ_LOGS_even_last
        #define SEQ_LOGS__odd_last
        #define SEQ_LOGS(args) EVAL(CAT(SEQ_LOGS__odd args, _last))

        #define SEQ_SIGN__odd(...) MAKE_SIGN __VA_ARGS__, SEQ_SIGN_even
        #define SEQ_SIGN_even(...) MAKE_SIGN __VA_ARGS__, SEQ_SIGN__odd
        #define SEQ_SIGN_even_last
        #define SEQ_SIGN__odd_last

        #define SEQ_NAME__odd(...) MAKE_NAME __VA_ARGS__, SEQ_NAME_even
        #define SEQ_NAME_even(...) MAKE_NAME __VA_ARGS__, SEQ_NAME__odd
        #define SEQ_NAME_even_last
        #define SEQ_NAME__odd_last

        #define SEQ_TYPE__odd(...) MAKE_TYPE __VA_ARGS__, SEQ_TYPE_even
        #define SEQ_TYPE_even(...) MAKE_TYPE __VA_ARGS__, SEQ_TYPE__odd
        #define SEQ_TYPE_even_last
        #define SEQ_TYPE__odd_last

    #if defined(_WIN32)
        #define SEQ_SIGN(args) EVAL(CAT(SEQ_SIGN__odd args, _last))
        #define SEQ_NAME(args) EVAL(CAT(SEQ_NAME__odd args, _last))
        #define SEQ_TYPE(args) EVAL(CAT(SEQ_TYPE__odd args, _last))
    #else
        #define DEL_(...)
        #define DEL_TAIL DEL_(
        #define SEQ_SIGN(args) SEQ_SIGN__odd args ((,DEL_TAIL ))) // Trailing comma workaround.
        #define SEQ_NAME(args) SEQ_NAME__odd args ((,DEL_TAIL ))) //
        #define SEQ_TYPE(args) SEQ_TYPE__odd args (( DEL_TAIL,))) //
    #endif

#endif
#if defined(MACROGEN_UNDEF)
    #undef MACROGEN_UNDEF

        #undef CAT_
        #undef CAT
        #undef EVAL_
        #undef EVAL
        #undef WRAP__odd
        #undef WRAP_even
        #undef WRAP_even_last
        #undef WRAP__odd_last
        #undef WRAP
        #undef MAKE_ATTR
        #undef MAKE_INIT
        #undef MAKE_SIGN
        #undef MAKE_NAME
        #undef MAKE_TYPE
        #undef SEQ_ATTR__odd
        #undef SEQ_ATTR_even
        #undef SEQ_ATTR_even_last
        #undef SEQ_ATTR__odd_last
        #undef SEQ_ATTR
        #undef SEQ_INIT__odd
        #undef SEQ_INIT_even
        #undef SEQ_INIT_even_last
        #undef SEQ_INIT__odd_last
        #undef SEQ_INIT
        #undef SEQ_SIGN__odd
        #undef SEQ_SIGN_even
        #undef SEQ_SIGN_even_last
        #undef SEQ_SIGN__odd_last
        #undef SEQ_NAME__odd
        #undef SEQ_NAME_even
        #undef SEQ_NAME_even_last
        #undef SEQ_NAME__odd_last
        #undef SEQ_TYPE__odd
        #undef SEQ_TYPE_even
        #undef SEQ_TYPE_even_last
        #undef SEQ_TYPE__odd_last
        #undef DEL_
        #undef DEL_AFTER
        #undef SEQ_SIGN
        #undef SEQ_NAME
        #undef SEQ_TYPE

#endif