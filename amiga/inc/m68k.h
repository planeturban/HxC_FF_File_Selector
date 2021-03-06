/*
 * m68k.h
 * 
 * M680x0 processor definitions.
 * 
 * Written & released by Keir Fraser <keir.xen@gmail.com>
 */

#ifndef __AMIGA_M68K_H__
#define __AMIGA_M68K_H__

typedef union {
    void *p;
    uint32_t x;
} m68k_vector_t;

struct m68k_vector_table {
    m68k_vector_t reset_ssp;
    m68k_vector_t reset_pc;
    m68k_vector_t bus_error;
    m68k_vector_t address_error;
    m68k_vector_t illegal_instruction;
    m68k_vector_t zero_divide;
    m68k_vector_t chk_chk2;
    m68k_vector_t trapcc_trapv;
    m68k_vector_t privilege_violation;
    m68k_vector_t trace;
    m68k_vector_t a_line;
    m68k_vector_t f_line;
    m68k_vector_t _rsvd0[2];
    m68k_vector_t format_error;
    m68k_vector_t uninitialised_interrupt;
    m68k_vector_t _rsvd1[8];
    m68k_vector_t spurious_interrupt;
    m68k_vector_t level1_autovector;
    m68k_vector_t level2_autovector;
    m68k_vector_t level3_autovector;
    m68k_vector_t level4_autovector;
    m68k_vector_t level5_autovector;
    m68k_vector_t level6_autovector;
    m68k_vector_t level7_autovector;
    m68k_vector_t trap[16];
    m68k_vector_t _rsvd2[16];
    m68k_vector_t user_interrupt[16];
};

#endif /* __AMIGA_M68K_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
