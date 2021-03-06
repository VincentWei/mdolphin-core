/*
** $Id: UcaseMg.h 123 2010-06-22 09:55:14Z xwyan $
**
** UcaseMg.h: unicode caseing implements.
**
** Copyright(C) 2007 Feynman Software.
**
** All rights reserved by Feynman Software.
**
** Create date: 2007-3-25
*/

#ifndef _UcaseMg_h
#define _UcaseMg_h

#include "unicode/UtypesMg.h"
#include "unicode/UcharMg.h"

struct UCaseProps;
typedef struct UCaseProps UCaseProps;

/**
 * Bit mask for getting just the options from a string compare options word
 * that are relevant for case folding (of a single string or code point).
 * See uchar.h.
 * @internal
 */
#define _FOLD_CASE_OPTIONS_MASK 0xff

/* complex/conditional mappings */
#define UCASE_EXC_CONDITIONAL_SPECIAL   0x4000
#define UCASE_EXC_CONDITIONAL_FOLD      0x8000

/* definitions for lengths word for full case mappings */
#define UCASE_FULL_LOWER    0xf
#define UCASE_FULL_FOLDING  0xf0
#define UCASE_FULL_UPPER    0xf00
#define UCASE_FULL_TITLE    0xf000

/* case-ignorable uses one of the delta bits, see gencase/store.c */
#define UCASE_CASE_IGNORABLE 0x40

/* string case mapping functions */

/**
 * Iterator function for string case mappings, which need to look at the
 * context (surrounding text) of a given character for conditional mappings.
 *
 * The iterator only needs to go backward or forward away from the
 * character in question. It does not use any indexes on this interface.
 * It does not support random access or an arbitrary change of
 * iteration direction.
 *
 * The code point being case-mapped itself is never returned by
 * this iterator.
 *
 * @param context A pointer to the iterator's working data.
 * @param dir If <0 then start iterating backward from the character;
 *            if >0 then start iterating forward from the character;
 *            if 0 then continue iterating in the current direction.
 * @return Next code point, or <0 when the iteration is done.
 */
typedef USChar32 UCaseContextIterator(void *context, int8_t dir);


/* definitions for 16-bit main exceptions word ------------------------------ */

/* first 8 bits indicate values in optional slots */
enum {
    UCASE_EXC_LOWER,
    UCASE_EXC_FOLD,
    UCASE_EXC_UPPER,
    UCASE_EXC_TITLE,
    UCASE_EXC_4,            /* reserved */
    UCASE_EXC_5,            /* reserved */
    UCASE_EXC_CLOSURE,
    UCASE_EXC_FULL_MAPPINGS,
    UCASE_EXC_ALL_SLOTS     /* one past the last slot */
};

/* each slot is 2 uint16_t instead of 1 */
#define UCASE_EXC_DOUBLE_SLOTS      0x100

/* reserved: exception bits 11..9 */

/* UCASE_EXC_DOT_MASK=UCASE_DOT_MASK<<UCASE_EXC_DOT_SHIFT */
#define UCASE_EXC_DOT_SHIFT     8

/* indexes into indexes[] */
enum {
    UCASE_IX_INDEX_TOP,
    UCASE_IX_LENGTH,
    UCASE_IX_TRIE_SIZE,
    UCASE_IX_EXC_LENGTH,
    UCASE_IX_UNFOLD_LENGTH,

    UCASE_IX_MAX_FULL_LENGTH=15,
    UCASE_IX_TOP=16
};

/* definitions for 16-bit case properties word ------------------------------ */

/* 2-bit constants for types of cased characters */
#define UCASE_TYPE_MASK     3
enum {
    UCASE_NONE,
    UCASE_LOWER,
    UCASE_UPPER,
    UCASE_TITLE
};

#define UCASE_GET_TYPE(props) ((props)&UCASE_TYPE_MASK)

#define UCASE_SENSITIVE     4
#define UCASE_EXCEPTION     8

#define UCASE_DOT_MASK      0x30

enum {
    UCASE_NO_DOT=0,         /* normal characters with cc=0 */
    UCASE_SOFT_DOTTED=0x10, /* soft-dotted characters with cc=0 */
    UCASE_ABOVE=0x20,       /* "above" accents with cc=230 */
    UCASE_OTHER_ACCENT=0x30 /* other accent character (0<cc!=230) */
};

/**
 * Sample struct which may be used by some implementations of
 * UCaseContextIterator.
 */
struct UCaseContext {
    void *p;
    int32_t start, index, limit;
    int32_t cpStart, cpLimit;
    int8_t dir;
    int8_t b1, b2, b3;
};
typedef struct UCaseContext UCaseContext;


enum {
    /**
     * For string case mappings, a single character (a code point) is mapped
     * either to itself (in which case in-place mapping functions do nothing),
     * or to another single code point, or to a string.
     * Aside from the string contents, these are indicated with a single int32_t
     * value as follows:
     *
     * Mapping to self: Negative values (~self instead of -self to support U+0000)
     *
     * Mapping to another code point: Positive values >UCASE_MAX_STRING_LENGTH
     *
     * Mapping to a string: The string length (0..UCASE_MAX_STRING_LENGTH) is
     * returned. Note that the string result may indeed have zero length.
     */
    UCASE_MAX_STRING_LENGTH=0x1f
};

/* no exception: bits 15..6 are a 10-bit signed case mapping delta */
#define UCASE_DELTA_SHIFT   6
#define UCASE_DELTA_MASK    0xffc0
#define UCASE_MAX_DELTA     0x1ff
#define UCASE_MIN_DELTA     (-UCASE_MAX_DELTA-1)
#define UCASE_EXC_SHIFT     4

#define UCASE_GET_DELTA(props) ((int16_t)(props)>>UCASE_DELTA_SHIFT)

#ifdef __cplusplus
extern "C" {
#endif

const UCaseProps* ucase_getSingleton(UErrorCode *pErrorCode);

/**
 * Get the full lowercase mapping for c.
 *
 * @param csp Case mapping properties.
 * @param c Character to be mapped.
 * @param iter Character iterator, used for context-sensitive mappings.
 *             See UCaseContextIterator for details.
 *             If iter==NULL then a context-independent result is returned.
 * @param context Pointer to be passed into iter.
 * @param pString If the mapping result is a string, then the pointer is
 *                written to *pString.
 * @param locale Locale ID for locale-dependent mappings.
 * @param locCache Initialize to 0; may be used to cache the result of parsing
 *                 the locale ID for subsequent calls.
 *                 Can be NULL.
 * @return Output code point or string length, see UCASE_MAX_STRING_LENGTH.
 *
 * @see UCaseContextIterator
 * @see UCASE_MAX_STRING_LENGTH
 * @internal
 */
int32_t ucase_toFullLower(const UCaseProps *csp, USChar32 c,
                  UCaseContextIterator *iter, void *context,
                  const UChar **pString,
                  const char *locale, int32_t *locCache);
int32_t ucase_toFullUpper(const UCaseProps *csp, USChar32 c,
                  UCaseContextIterator *iter, void *context,
                  const UChar **pString,
                  const char *locale, int32_t *locCache);
int32_t ucase_hasBinaryProperty(USChar32 c, UProperty which);
int32_t ucase_toFullFolding(const UCaseProps *csp, USChar32 c,
                    const UChar **pString,
                    uint32_t options);

#ifdef  __cplusplus
}
#endif

#endif
