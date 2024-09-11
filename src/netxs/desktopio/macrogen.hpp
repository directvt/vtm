// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#if defined(DEFINE_macro)
    #undef DEFINE_macro

        #define CAT_macro_(x, ...) x ## __VA_ARGS__
        #define CAT_macro(x, ...) CAT_macro_(x, __VA_ARGS__)

        #define EVAL_macro_(...) __VA_ARGS__
        #define EVAL_macro(...) EVAL_macro_(__VA_ARGS__)

        #define WRAP__odd(...) ((__VA_ARGS__))WRAP_even
        #define WRAP_even(...) ((__VA_ARGS__))WRAP__odd
        #define WRAP_even_last
        #define WRAP__odd_last
        #define WRAP_macro(args) EVAL_macro(CAT_macro(WRAP__odd args, _last))

        #define TAKE_NAME_macro(...) __VA_OPT__(this->__VA_ARGS__) // Ignore trailing spaces.
        #define TAKE_TEMP_macro(...) __VA_OPT__(_ ## __VA_ARGS__) // gcc don't get _##name inline.
        #define MAKE_NAME_macro(type, name, ...) TAKE_NAME_macro(name)
        #define MAKE_INIT_macro(type, name, ...) this->name = TAKE_TEMP_macro(name);
        #define MAKE_ATTR_macro(type, name, ...) type name{};
        #define MAKE_SIGN_macro(type, name, ...) type TAKE_TEMP_macro(name)
        #define MAKE_TYPE_macro(type, name, ...) type
        #define MAKE_TEMP_macro(type, name, ...) this->name = source. name;
        #define MAKE_LOGS_macro(type, name, ...) s << "\n\t " << #name << ": " << o.name;
        #define MAKE_WIPE_macro(type, name, ...) this->name = {};

        #define SEQ_ATTR__odd(...) MAKE_ATTR_macro __VA_ARGS__ SEQ_ATTR_even
        #define SEQ_ATTR_even(...) MAKE_ATTR_macro __VA_ARGS__ SEQ_ATTR__odd
        #define SEQ_ATTR_even_last
        #define SEQ_ATTR__odd_last
        #define SEQ_ATTR_macro(args) EVAL_macro(CAT_macro(SEQ_ATTR__odd args, _last))

        #define SEQ_INIT__odd(...) MAKE_INIT_macro __VA_ARGS__ SEQ_INIT_even
        #define SEQ_INIT_even(...) MAKE_INIT_macro __VA_ARGS__ SEQ_INIT__odd
        #define SEQ_INIT_even_last
        #define SEQ_INIT__odd_last
        #define SEQ_INIT_macro(args) EVAL_macro(CAT_macro(SEQ_INIT__odd args, _last))

        #define SEQ_TEMP__odd(...) MAKE_TEMP_macro __VA_ARGS__ SEQ_TEMP_even
        #define SEQ_TEMP_even(...) MAKE_TEMP_macro __VA_ARGS__ SEQ_TEMP__odd
        #define SEQ_TEMP_even_last
        #define SEQ_TEMP__odd_last
        #define SEQ_TEMP_macro(args) EVAL_macro(CAT_macro(SEQ_TEMP__odd args, _last))

        #define SEQ_WIPE__odd(...) MAKE_WIPE_macro __VA_ARGS__ SEQ_WIPE_even
        #define SEQ_WIPE_even(...) MAKE_WIPE_macro __VA_ARGS__ SEQ_WIPE__odd
        #define SEQ_WIPE_even_last
        #define SEQ_WIPE__odd_last
        #define SEQ_WIPE_macro(args) EVAL_macro(CAT_macro(SEQ_WIPE__odd args, _last))

        #define SEQ_LOGS__odd(...) MAKE_LOGS_macro __VA_ARGS__ SEQ_LOGS_even
        #define SEQ_LOGS_even(...) MAKE_LOGS_macro __VA_ARGS__ SEQ_LOGS__odd
        #define SEQ_LOGS_even_last
        #define SEQ_LOGS__odd_last
        #define SEQ_LOGS_macro(args) EVAL_macro(CAT_macro(SEQ_LOGS__odd args, _last))

        #define SEQ_SIGN__odd(...) MAKE_SIGN_macro __VA_ARGS__, SEQ_SIGN_even
        #define SEQ_SIGN_even(...) MAKE_SIGN_macro __VA_ARGS__, SEQ_SIGN__odd
        #define SEQ_SIGN_even_last
        #define SEQ_SIGN__odd_last

        #define SEQ_NAME__odd(...) MAKE_NAME_macro __VA_ARGS__, SEQ_NAME_even
        #define SEQ_NAME_even(...) MAKE_NAME_macro __VA_ARGS__, SEQ_NAME__odd
        #define SEQ_NAME_even_last
        #define SEQ_NAME__odd_last

        #define SEQ_TYPE__odd(...) MAKE_TYPE_macro __VA_ARGS__, SEQ_TYPE_even
        #define SEQ_TYPE_even(...) MAKE_TYPE_macro __VA_ARGS__, SEQ_TYPE__odd
        #define SEQ_TYPE_even_last
        #define SEQ_TYPE__odd_last

        #define DEL_macro_(...)
        #define DEL_TAIL_macro DEL_macro_(
        #define SEQ_SIGN_macro(args) SEQ_SIGN__odd args ((,DEL_TAIL_macro ))) // Trailing comma workaround.
        #define SEQ_NAME_macro(args) SEQ_NAME__odd args ((,DEL_TAIL_macro ))) //
        #define SEQ_TYPE_macro(args) SEQ_TYPE__odd args (( DEL_TAIL_macro,))) //

#endif
#if defined(UNDEFINE_macro)
    #undef UNDEFINE_macro

        #undef CAT_macro_
        #undef CAT_macro
        #undef EVAL_macro_
        #undef EVAL_macro
        #undef WRAP__odd
        #undef WRAP_even
        #undef WRAP_even_last
        #undef WRAP__odd_last
        #undef WRAP_macro
        #undef MAKE_ATTR_macro
        #undef MAKE_INIT_macro
        #undef MAKE_SIGN_macro
        #undef MAKE_NAME_macro
        #undef MAKE_TYPE_macro
        #undef SEQ_ATTR__odd
        #undef SEQ_ATTR_even
        #undef SEQ_ATTR_even_last
        #undef SEQ_ATTR__odd_last
        #undef SEQ_ATTR_macro
        #undef SEQ_INIT__odd
        #undef SEQ_INIT_even
        #undef SEQ_INIT_even_last
        #undef SEQ_INIT__odd_last
        #undef SEQ_INIT_macro
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
        #undef DEL_macro_
        #undef DEL_TAIL_macro
        #undef SEQ_SIGN_macro
        #undef SEQ_NAME_macro
        #undef SEQ_TYPE_macro

#endif