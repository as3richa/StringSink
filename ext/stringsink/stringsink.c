#include <ruby.h>

/* ============================================================
 * Types
 * ============================================================ */

/* A stringsink is layed out in memory as a stringsink_header_t
 * immediately followed by a sequence of (possibly zero) characters.
 * This saves a level of indirection */

typedef struct {
  size_t length;
  size_t capacity;
} stringsink_header_t;

/* ============================================================
 * Constants
 * ============================================================ */

/* class StringSink; end */
static VALUE stringsink_class;

/* to_s_sym = :to_s
 * linefeed = "\n" */
ID to_s_sym;
VALUE linefeed;

/* For stringsink_data_type */
static void stringsink_free(void* header);
static size_t stringsink_size(const void* header);

static const rb_data_type_t stringsink_data_type = {
  .wrap_struct_name = "StringSink",
  .function = {
    .dfree = stringsink_free,
    .dsize = stringsink_size,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

/* ============================================================
 * Utility functions and macros
 * ============================================================ */

#ifndef STRINGSINK_MAX_LENGTH

  #include <limits.h>
  #define STRINGSINK_MAX_LENGTH SIZE_T_MAX

#endif

#ifdef STRINGSINK_DEBUG

  #undef NDEBUG
  #include <assert.h>

#else

  #define assert(cond)

#endif


#define STRINGSINK_SIZE_FOR_CAPACITY(capacity) \
  (sizeof(stringsink_header_t) + capacity)

#define STRINGSINK_BUFFER(header) \
  (((char*)header) + sizeof(stringsink_header_t))

#define STRINGSINK_UNPACK(value, header) \
  (header) = rb_check_typeddata(value, &stringsink_data_type)

static void stringsink_free(void* header) {
  ruby_xfree(header);
}

static size_t stringsink_size(const void* header) {
  const size_t capacity = ((stringsink_header_t*)header)->capacity;
  return STRINGSINK_SIZE_FOR_CAPACITY(capacity);
}

static inline void stringsink_resize(VALUE self, stringsink_header_t** header, size_t capacity) {
  assert((*header)->length <= (*header)->capacity);
  assert((*header)->length <= capacity);

  (*header) = ruby_xrealloc((*header), STRINGSINK_SIZE_FOR_CAPACITY(capacity));
  assert((*header) != NULL);

  (*header)->capacity = capacity;
  RTYPEDDATA_DATA(self) = (*header);
}

static inline void stringsink_resize_for_growth(VALUE self, stringsink_header_t** header, size_t growth) {
  assert((*header)->length <= (*header)->capacity);

  if(growth > STRINGSINK_MAX_LENGTH - (*header)->length) {
    rb_raise(rb_eArgError, "StringSink is too large");
  }

  const size_t length = growth + (*header)->length;
  const size_t prev_capacity = (*header)->capacity;

  if(length <= prev_capacity) {
    return;
  }

  size_t capacity;

  if(prev_capacity > STRINGSINK_MAX_LENGTH / 2) {
    capacity = STRINGSINK_MAX_LENGTH;
  } else {
    capacity = (2 * prev_capacity >= length) ? (2 * prev_capacity) : length;
  }

  assert(length <= capacity);

  stringsink_resize(self, header, capacity);
}

static inline void stringsink_concat(stringsink_header_t* header, const char* buffer, size_t length) {
  assert(header->length + length <= header->capacity);

  if(length == 0) {
    return;
  }

  memcpy(STRINGSINK_BUFFER(header) + header->length, buffer, length);
  header->length += length;
}

/* ============================================================
 * Public API
 * ============================================================ */

static VALUE stringsink_allocate(VALUE self) {
  const size_t size = STRINGSINK_SIZE_FOR_CAPACITY(0);

  void* pointer;
  const VALUE ss = rb_data_typed_object_make(self, &stringsink_data_type, &pointer, size);

  stringsink_header_t* header = pointer;
  header->length = 0;
  header->capacity = 0;

  return ss;
}

static VALUE stringsink_write(VALUE self, VALUE string) {
  string = rb_obj_as_string(string);

  stringsink_header_t* header;
  STRINGSINK_UNPACK(self, header);

  const char* const buffer = RSTRING_PTR(string);
  const size_t length = RSTRING_LEN(string);

  stringsink_resize_for_growth(self, &header, length);
  stringsink_concat(header, buffer, length);

  return SIZET2NUM(length);
}

static VALUE stringsink_shift(VALUE self, VALUE string) {
  stringsink_write(self, string);
  return self;
}

static VALUE stringsink_print(int argc, VALUE* argv, VALUE self) {
  size_t growth = 0;

  for(int i = 0; i < argc; i ++) {
    argv[i] = rb_obj_as_string(argv[i]);
    growth += RSTRING_LEN(argv[i]);
  }

  stringsink_header_t* header;
  STRINGSINK_UNPACK(self, header);

  stringsink_resize_for_growth(self, &header, growth);

  for(int i = 0; i < argc; i ++) {
    stringsink_concat(header, RSTRING_PTR(argv[i]), RSTRING_LEN(argv[i]));
  }

  return Qnil;
}

static VALUE stringsink_puts(int argc, VALUE* argv, VALUE self) {
  if(argc == 0) {
    stringsink_write(self, linefeed);
    return Qnil;
  }

  VALUE* argv2 = ruby_xmalloc(sizeof(VALUE) * (2 * argc));

  int k = 0;

  for(int i = 0; i < argc; i ++) {
    argv2[k ++] = argv[i];

    const size_t length = RSTRING_LEN(argv[i]);

    if(length == 0 || RSTRING_PTR(argv[i])[length - 1] != '\n') {
      argv2[k ++] = linefeed;
    }
  }

  stringsink_print(k, argv2, self);

  ruby_xfree(argv2);

  return Qnil;
}

static VALUE stringsink_printf(int argc, VALUE* argv, VALUE self) {
  const VALUE string = rb_f_sprintf(argc, argv);
  stringsink_write(self, string);
  return Qnil;
}

static VALUE stringsink_putc(VALUE self, VALUE ch) {
  if (RB_TYPE_P(ch, T_STRING)) {
    stringsink_write(self, rb_str_substr(ch, 0, 1));
  } else {
    char c = NUM2CHR(ch);

    stringsink_header_t* header;
    STRINGSINK_UNPACK(self, header);

    stringsink_resize_for_growth(self, &header, 1);
    STRINGSINK_BUFFER(header)[header->length ++] = c;
  }

  return ch;
}

static VALUE stringsink_to_s_utf8(VALUE self) {
  stringsink_header_t* header;
  STRINGSINK_UNPACK(self, header);
  return rb_utf8_str_new(STRINGSINK_BUFFER(header), header->length);
}

static VALUE stringsink_shrink_bang(VALUE self) {
  stringsink_header_t* header;
  STRINGSINK_UNPACK(self, header);

  stringsink_resize(self, &header, header->length);

  return self;
}

/* ============================================================
 * Extension init
 * ============================================================ */

void Init_stringsink(void) {
  /* StringSink class */
  stringsink_class = rb_define_class("StringSink", rb_cData);

  /* Version */
  const VALUE version_string = rb_str_new_literal("0.1.0");
  rb_define_const(stringsink_class, "VERSION", version_string);

  /* Allocator */
  rb_define_alloc_func(stringsink_class, stringsink_allocate);

  /* Public instance methods */
  rb_define_method(stringsink_class, "write",   stringsink_write,       1);
  rb_define_method(stringsink_class, "<<",      stringsink_shift,       1);
  rb_define_method(stringsink_class, "print",   stringsink_print,      -1);
  rb_define_method(stringsink_class, "printf",  stringsink_printf,     -1);
  rb_define_method(stringsink_class, "puts",    stringsink_puts,       -1);
  rb_define_method(stringsink_class, "putc",    stringsink_putc,        1);
  rb_define_method(stringsink_class, "to_s",    stringsink_to_s_utf8,   0);
  rb_define_method(stringsink_class, "string",  stringsink_to_s_utf8,   0);
  rb_define_method(stringsink_class, "shrink!", stringsink_shrink_bang, 0);

  /* Useful constants */
  linefeed = rb_str_new_literal("\n");

  /* Prevent linefeed from getting GC'ed */
  rb_define_const(stringsink_class, "LINEFEED", linefeed);
}
