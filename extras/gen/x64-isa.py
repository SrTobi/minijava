from email.utils import formatdate
import argparse
import sys

PROGRAM = 'x64-isa.py'

TIMESTAMP = formatdate()

# The instructions and their descriptions are copied from chapter 5
# (Instruction Set Summary) of the "Intel (R) 64 and IA-32 Architectures
# Software Developer's Manual", volume 1 (Basic Architecture).  The section
# numbers in the comments below refer to this publication.  If you find
# instructions missing, please always copy entire sections and keep their
# order, even if you think that some instructions therein might not be needed.
# Otherwise, this collection will quickly become a mess.

MACROS = [
    ("CALL_ALIGNED", "Call a function with correct stack pointer alignment (macro)"),
    ("DIV", "Compute quotient of two registers (macro)"),
    ("MOD", "Compute remainder of division of two registers (macro)"),
]

INSTRUCTIONS = [

    # 5.1 General Purpose Instructions

    # 5.1.1 Data transfer instructions

    ('MOV',       "Move data between general-purpose registers; move data between memory and general-purpose or segment registers; move immediates to general-purpose registers"),
    ('CMOVE',     "Conditional move if equal"),
    ('CMOVZ',     "Conditional move if zero"),
    ('CMOVNE',    "Conditional move if not equal"),
    ('CMOVNZ',    "Conditional move if not zero"),
    ('CMOVA',     "Conditional move if above"),
    ('CMOVNBE',   "Conditional move if not below or equal"),
    ('CMOVAE',    "Conditional move if above or equal"),
    ('CMOVNB',    "Conditional move if not below"),
    ('CMOVB',     "Conditional move if below"),
    ('CMOVNAE',   "Conditional move if not above or equal"),
    ('CMOVBE',    "Conditional move if below or equal"),
    ('CMOVNA',    "Conditional move if not above"),
    ('CMOVG',     "Conditional move if greater"),
    ('CMOVNLE',   "Conditional move if not less or equal"),
    ('CMOVGE',    "Conditional move if greater or equal"),
    ('CMOVNL',    "Conditional move if not less"),
    ('CMOVL',     "Conditional move if less"),
    ('CMOVNGE',   "Conditional move if not greater or equal"),
    ('CMOVLE',    "Conditional move if less or equal"),
    ('CMOVNG',    "Conditional move if not greater"),
    ('CMOVC',     "Conditional move if carry"),
    ('CMOVNC',    "Conditional move if not carry"),
    ('CMOVO',     "Conditional move if overflow"),
    ('CMOVNO',    "Conditional move if not overflow"),
    ('CMOVS',     "Conditional move if sign (negative)"),
    ('CMOVNS',    "Conditional move if not sign (non-negative)"),
    ('CMOVP',     "Conditional move if parity"),
    ('CMOVPE',    "Conditional move if parity even"),
    ('CMOVNP',    "Conditional move if not parity"),
    ('CMOVPO',    "Conditional move if parity odd"),
    ('XCHG',      "Exchange"),
    ('BSWAP',     "Byte swap"),
    ('XADD',      "Exchange and add"),
    ('CMPXCHG',   "Compare and exchange"),
    ('CMPXCHG8B', "Compare and exchange 8 bytes"),
    ('PUSH',      "Push onto stack"),
    ('POP',       "Pop off of stack"),
    ('PUSHA',     "Push general-purpose registers onto stack"),
    ('PUSHAD',    "Push general-purpose registers onto stack"),
    ('POPA',      "Pop general-purpose registers from stack"),
    ('POPAD',     "Pop general-purpose registers from stack"),
    ('CWD',       "Convert word to doubleword"),
    ('CDQ',       "Convert doubleword to quadword"),
    ('CBW',       "Convert byte to word"),
    ('CWDE',      "Convert word to doubleword in EAX register"),
    ('CQO',       "Sign-extend quad in RAX to octuple in RDX:RAX (x86-64 only)"),
    ('MOVSLQ',    "Move and sign extend 32 bit to 64 bit register"),

    # 5.1.2 Binary Arithmetic Instructions

    ('ADCX',      "Unsigned integer add with carry"),
    ('ADOX',      "Unsigned integer add with overflow"),
    ('ADD',       "Integer add"),
    ('ADC',       "Add with carry"),
    ('SUB',       "Subtract"),
    ('SBB',       "Subtract with borrow"),
    ('IMUL',      "Signed multiply"),
    ('MUL',       "Unsigned multiply"),
    ('IDIV',      "Signed divide"),
    ('DIV',       "Unsigned divide"),
    ('INC',       "Increment"),
    ('DEC',       "Decrement"),
    ('NEG',       "Negate"),
    ('CMP',       "Compare"),

    # 5.1.3 Decimal Arithmetic Instructions (skipped)

    # 5.1.4 Logical Instructions

    ('AND',       "Perform bitwise logical AND"),
    ('OR',        "Perform bitwise logical OR"),
    ('XOR',       "Perform bitwise logical exclusive OR"),
    ('NOT',       "Perform bitwise logical NOT"),

    # 5.1.5 Shift and Rotate Instructions

    ('SAR',       "Shift arithmetic right"),
    ('SHR',       "Shift logical right"),
    ('SAL',       "Shift arithmetic left"),
    ('SHL',       "Shift logical left"),
    ('SHRD',      "Shift right double"),
    ('SHLD',      "Shift left double"),
    ('ROR',       "Rotate right"),
    ('ROL',       "Rotate left"),
    ('RCR',       "Rotate through carry right"),
    ('RCL',       "Rotate through carry left"),

    # 5.1.6 Bit and Byte Instructions

    ('BT',        "Bit test"),
    ('BTS',       "Bit test and set"),
    ('BTR',       "Bit test and reset"),
    ('BTC',       "Bit test and complement"),
    ('BSF',       "Bit scan forward"),
    ('BSR',       "Bit scan reverse"),
    ('SETE',      "Set byte if equal"),
    ('SETZ',      "Set byte if zero"),
    ('SETNE',     "Set byte if not equal"),
    ('SETNZ',     "Set byte if not zero"),
    ('SETA',      "Set byte if above"),
    ('SETNBE',    "Set byte if not below or equal"),
    ('SETAE',     "Set byte if above or equal"),
    ('SETNB',     "Set byte if not below"),
    ('SETNC',     "Set byte if not carry"),
    ('SETB',      "Set byte if below"),
    ('SETNAE',    "Set byte if not above or equal"),
    ('SETC',      "Set byte if carry"),
    ('SETBE',     "Set byte if below or equal"),
    ('SETNA',     "Set byte if not above"),
    ('SETG',      "Set byte if greater"),
    ('SETNLE',    "Set byte if not less or equal"),
    ('SETGE',     "Set byte if greater or equal"),
    ('SETNL',     "Set byte if not less"),
    ('SETL',      "Set byte if less"),
    ('SETNGE',    "Set byte if not greater or equal"),
    ('SETLE',     "Set byte if less or equal"),
    ('SETNG',     "Set byte if not greater"),
    ('SETS',      "Set byte if sign (negative)"),
    ('SETNS',     "Set byte if not sign (non-negative)"),
    ('SETO',      "Set byte if overflow"),
    ('SETNO',     "Set byte if not overflow"),
    ('SETPE',     "Set byte if parity even"),
    ('SETP',      "Set byte if parity"),
    ('SETPO',     "Set byte if parity odd"),
    ('SETNP',     "Set byte if not parity"),
    ('TEST',      "Logical compare"),
    ('CRC32',     "Provides hardware acceleration to calculate cyclic redundancy checks for fast and efficient implementation of data integrity protocols"),
    ('POPCNT',    "This instruction calculates of number of bits set to 1 in the second operand (source) and returns the count in the first operand (a destination register)"),

    # 5.1.7 Control Transfer Instructions

    ('JMP',       "Jump"),
    ('JE',        "Jump if equal"),
    ('JZ',        "Jump if zero"),
    ('JNE',       "Jump if not equal"),
    ('JNZ',       "Jump if not zero"),
    ('JA',        "Jump if above"),
    ('JNBE',      "Jump if not below or equal"),
    ('JAE',       "Jump if above or equal"),
    ('JNB',       "Jump if not below"),
    ('JB',        "Jump if below"),
    ('JNAE',      "Jump if not above or equal"),
    ('JBE',       "Jump if below or equal"),
    ('JNA',       "Jump if not above"),
    ('JG',        "Jump if greater"),
    ('JNLE',      "Jump if not less or equal"),
    ('JGE',       "Jump if greater or equal"),
    ('JNL',       "Jump if not less"),
    ('JL',        "Jump if less"),
    ('JNGE',      "Jump if not greater or equal"),
    ('JLE',       "Jump if less or equal"),
    ('JNG',       "Jump if not greater"),
    ('JC',        "Jump if carry"),
    ('JNC',       "Jump if not carry"),
    ('JO',        "Jump if overflow"),
    ('JNO',       "Jump if not overflow"),
    ('JS',        "Jump if sign (negative)"),
    ('JNS',       "Jump if not sign (non-negative)"),
    ('JPO',       "Jump if parity odd"),
    ('JNP',       "Jump if not parity"),
    ('JPE',       "Jump if parity even"),
    ('JP',        "Jump if parity"),
    ('JCXZ',      "Jump register CX zero"),
    ('JECXZ',     "Jump register ECX zero"),
    ('LOOP',      "Loop with ECX counter"),
    ('LOOPZ',     "Loop with ECX and zero"),
    ('LOOPE',     "Loop with ECX and equal"),
    ('LOOPNZ',    "Loop with ECX and not zero"),
    ('LOOPNE',    "Loop with ECX and not equal"),
    ('CALL',      "Call procedure"),
    ('RET',       "Return"),
    ('IRET',      "Return from interrupt"),
    ('INT',       "Software interrupt"),
    ('INTO',      "Interrupt on overflow"),
    ('BOUND',     "Detect value out of range"),
    ('ENTER',     "High-level procedure entry"),
    ('LEAVE',     "High-level procedure exit"),

    # 5.1.8 String Instructions (skipped)

    # 5.1.9. I/O Instructions (skipped)

    # 5.1.10 Enter and Leave Instructions (skipped)

    # 5.1.11 Flag Control (EFLAG) Instructions

    ('STC',       "Set carry flag"),
    ('CLC',       "Clear the carry flag"),
    ('CMC',       "Complement the carry flag"),
    ('CLD',       "Clear the direction flag"),
    ('STD',       "Set direction flag"),
    ('LAHF',      "Load flags into AH register"),
    ('SAHF',      "Store AH register into flags"),
    ('PUSHF',     "Push EFLAGS onto stack"),
    ('PUSHFD',    "Push EFLAGS onto stack"),
    ('POPF',      "Pop EFLAGS from stack"),
    ('POPFD',     "Pop EFLAGS from stack"),
    ('STI',       "Set interrupt flag"),
    ('CLI',       "Clear the interrupt flag"),

    # 5.1.12 Segment Register Instructions (skipped)

    # 5.1.13 Miscellaneous Instructions

    ('LEA',         "Load effective address"),
    ('NOP',         "No operation"),
    ('UD2',         "Undefined instruction"),
    ('XLAT',        "Table lookup translation"),
    ('XLATB',       "Table lookup translation"),
    ('CPUID',       "Processor identification"),
    ('MOVBE',       "Move data after swapping data bytes"),
    ('PREFETCHW',   "Prefetch data into cache in anticipation of write"),
    ('PREFETCHWT1', "Prefetch hint T1 with intent to write"),
    ('CLFLUSH',     "Flushes and invalidates a memory operand and its associated cache line from all levels of the processor's cache hierarchy"),
    ('CLFLUSHOPT',  "Flushes and invalidates a memory operand and its associated cache line from all levels of the processor's cache hierarchy with optimized memory system throughput"),

    # 5.1.14 User Mode Extended Sate Save / Restore Instructions (skipped)

    # 5.1.15 Random Number Generator Instructions (skipped)

    # 5.1.16 BMI1, BMI2

    ('ANDN',        "Bitwise AND of first source with inverted 2nd source operands"),
    ('BEXTR',       "Contiguous bitwise extract"),
    ('BLSI',        "Extract lowest set bit"),
    ('BLSMSK',      "Set all lower bits below first set bit to 1"),
    ('BLSR',        "Reset lowest set bit"),
    ('BZHI',        "Zero high bits starting from specified bit position"),
    ('LZCNT',       "Count the number leading zero bits"),
    ('MULX',        "Unsigned multiply without affecting arithmetic flags"),
    ('PDEP',        "Parallel deposit of bits using a mask"),
    ('PEXT',        "Parallel extraction of bits using a mask"),
    ('RORX',        "Rotate right without affecting arithmetic flags"),
    ('SARX',        "Shift arithmetic right"),
    ('SHLX',        "Shift logic left"),
    ('SHRX',        "Shift logic right"),
    ('TZCNT',       "Count the number trailing zero bits"),

]


def print_header_file():
    print_noedit_comment()
    print("""
#pragma once

/**
 * @file opcode.hpp
 *
 * @brief
 *     List of x64 instructions.
 *
 */
namespace minijava
{
\tnamespace backend
\t{
\t\t/**
\t\t * @brief
\t\t *     General-purpose x64 instructions (including macros).
\t\t *
\t\t */
\t\tenum class opcode
\t\t{""")
    print('\t\t\t{:24s}{:s}'.format('none = 0,', '///< No instruction'))
    for (prefix, codeset) in [('mac', MACROS), ('op', INSTRUCTIONS)]:
        for (mnemotic, description) in codeset:
            enumerator = prefix + '_' + mnemotic.lower()
            docstring = '///< {:s}'.format(description)
            print('\t\t\t{:24s}{:s}'.format(enumerator + ',', docstring))
    print("""\t\t};

\t\t/**
\t\t * @brief
\t\t *     `return`s the mnemotic for an x64 general-purpose instruction.
\t\t *
\t\t * If `op` is `none` or not a declared enumerator, `nullptr` is `return`ed.
\t\t *
\t\t * The `return`ed pointer refers to a statically allocated buffer of read-only data.
\t\t *
\t\t * @param op
\t\t *     general-purpose instruction
\t\t *
\t\t * @returns
\t\t *     mnemotic for the instruction
\t\t *
\t\t */
\t\tconstexpr const char* mnemotic(const opcode op) noexcept
\t\t{
\t\t\t// TODO: Replace the `switch` with a more efficient table lookup
\t\t\tswitch (op) {""")
    for (prefix, asm_prefix, codeset) in [('mac', 'mj_', MACROS), ('op', '', INSTRUCTIONS)]:
        for (mnemotic, description) in codeset:
            col1 = 'case opcode::{:s}_{:s}:'.format(prefix, mnemotic.lower())
            col2 = 'return "{:s}{:s}";'.format(asm_prefix, mnemotic.lower())
            print('\t\t\t{:32s}{:s}'.format(col1, col2))
    print('\t\t\t{:32s}{:s}'.format('default:', 'return nullptr;'))
    print("""\t\t\t}
\t\t}

\t}  // namespace backend
}  // namespace minijava""")

def print_noedit_comment():
    def outhdr(text=None):
        if text is None:
            text = '-' * 70
        print('/* {:^70s} */'.format(text))
    outhdr()
    outhdr("")
    outhdr("THIS IS A GENERATED FILE, DO NOT EDIT!")
    outhdr("")
    outhdr("Generated by '{}' on {}".format(PROGRAM, TIMESTAMP))
    outhdr("")
    outhdr()

def smart_open(filename=None):
    if filename is None or filename == '-':
        return sys.stdout
    return open(filename, 'w')

if __name__ == '__main__':
    ap = argparse.ArgumentParser(
        prog=PROGRAM,
        description="Generates C++ code for the list of x64 instructions."
    )
    ap.add_argument(
        '-o', '--output', metavar='FILE', type=str,
        help="redirect output to FILE"
    )
    ns = ap.parse_args()
    with smart_open(ns.output) as sys.stdout:
        print_header_file()
