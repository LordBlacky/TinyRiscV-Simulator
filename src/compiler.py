import os
import re

# this is just the enum copied from the simulator
instructions_list = [
    "EMPTY", "ADD", "SUB", "AND", "OR", "XOR", "SLT", "SLTU", "SRA", "SRL", "SLL", "MUL", "SLLI",
    "ADDI", "ANDI", "ORI", "XORI", "SLTI", "SLTIU", "SRAI", "SRLI", "LUI", "AUIPC",
    "LW", "SW", "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU", "JAL", "JALR", "FLAG",
    "NOP", "LI", "LA", "MV", "NOT", "NEG", "SEQZ", "SNEZ", "SLTZ", "SGTZ", "BEQZ", "BNEZ", "BLEZ", "BGEZ", "BLTZ", "BGTZ",
    "BGT", "BLE", "BGTU", "BLEU", "J", "JR", "RET","CALL"]

instructions_dict = {}


def concatenate_files(output_file, *input_files):
    """
    Concatenates multiple text files into one output file.

    Args:
        output_file (str): The path of the output file.
        *input_files (str): Paths of the input files to concatenate.

    Returns:
        list of tuples: A list of tuples, where each tuple contains the start and
                        end line numbers for each input file in the output file.
    """
    line_ranges = []
    current_line = 1

    with open(output_file, 'w') as outfile:
        for input_file in input_files:
            with open(input_file, 'r') as infile:
                lines = infile.readlines()
                start_line = current_line
                outfile.writelines(lines)
                current_line += len(lines)
                end_line = current_line - 1

                # Append the start and end line numbers for the current file
                line_ranges.append((start_line, end_line))

    return line_ranges


def expand_macros(file):
    lines = []
    with open(file, 'r') as infile:
        lines = infile.readlines()
        count = len(lines)

    # create macro data
    macros = {}
    current_macro_name = ''
    for i in range(count):
        s = lines[i]
        # add lines if we are in a macro
        if current_macro_name != '':
            if (re.match(r"^\s*\.endm", s)):
                current_macro_name = ''
            else:
                macros[current_macro_name] += s

            lines[i] = ""
        # otherwise check for new macro
        else:
            mac = re.match(r"^\s*\.macro (\w+)\s*$", s)
            if (mac is not None):
                if macros.__contains__(mac.group(1)):
                    print("two identical macros!!! for:", mac.group(1))
                current_macro_name = mac.group(1)
                macros[current_macro_name] = ""
                print("macro: ", current_macro_name)
                lines[i] = ""

    # expand macros
    for i in range(count):
        if re.match(r"^\s*\w+\s*$", lines[i]):
            s = lines[i].strip()
            if macros.__contains__(s):
                lines[i] = macros[s]

    # write File
    with open(file, 'w') as outfile:
        outfile.writelines(lines)


def compile(file):
    """
    removes comments and replace labels with actuall immediates
    also adds a JAL instruction to the lable _start (still todo)
    then replace the instructions and argumends with ids
    """
    lines = []
    with open(file, 'r') as infile:
        lines = infile.readlines()
        count = len(lines)

    # remove comments and sections
    for i in range(count):
        s = lines[i]
        s = re.sub(r"\s*;.*$", "", s)
        s = re.sub(r"\s*#.*$", "", s)
        s = re.sub(r"\s*\..*$", "", s)
        lines[i] = s

    # determine _start line
    startline = 0
    for i in range(count):
        if (re.search(r"^\s*_start:\s*$", lines[i]) is not None):
            startline = i
            break

    # make label dict containing lable as key and linenumber as value
    # also make lable def line a blank line
    labels = {}
    for i in range(count):
        this_lab = re.match(r"\s*(\w+):\s$", lines[i])
        if (this_lab is not None):
            this_lab = this_lab.group(1)
            labels[this_lab] = i
            lines[i] = "\n"

    # print(lines)
    # iterate through lines and replace everything with ids
    for i in range(count):
        parts = re.split(r",? |,|\(", lines[i].strip())
        # if bracket syntax is used (e.g "jalr x1 0(x2)"):
        # swap second and third argument
        if (re.match(r"\d\(.+\)", lines[i])):
            t = parts[1]
            parts[1] = parts[2]
            parts[2] = t

        if (parts[0] == ''):
            parts[0] = "EMPTY"  # to fill empty lines with zeros

        out_line = str(get_inst_id(parts[0]))

        # fill 3 args with Zero
        while len(parts) < 4:
            parts.append("0")

        for j in range(1, 4):
            out_line += " " + str(get_arg_id(parts[j], labels, i))

        lines[i] = out_line + "\n"

    # write File
    with open(file, 'w') as outfile:
        outfile.writelines(lines)

    # print(lines)


alias_list = ["zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4",
              "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"]

alias_dict = {}


def get_arg_id(arg: str, labels: dict, current_line):
    # check if its a register
    alias_arg = re.match(r"([^\)]*)\)?", arg).group(1)
    # print("alias arg: ", alias_arg, " normal arg: ", arg)
    if (alias_arg is not None and alias_arg in alias_dict):
        return int(alias_dict[alias_arg])

    r = re.match(r"x(\d\d?)", arg)
    # print(r)
    if (r):
        return int(r.group(1))

    # check if its a immediate
    if (re.match(r"^-?\d+$", arg)):
        return int(arg)
    elif (re.match(r"0x[0-9a-fA-F]+", arg)):  # number is hex Value
        return int(arg.lower(), 0)

        # check if its a label
    if (labels.__contains__(arg)):
        return (labels[arg] - current_line) * 4

    print(f"Error: '{arg}' can't be resolved")


def get_inst_id(arg: str):
    arg = arg.upper()
    return instructions_dict[arg]


def init_dict(dest_dict, in_list):
    for i in range(len(in_list)):
        dest_dict[in_list[i]] = i


def init_alias_dict():
    init_dict(alias_dict, alias_list)


def init_instruction_dict():
    init_dict(instructions_dict, instructions_list)


if __name__ == "__main__":

    init_instruction_dict()
    init_alias_dict()

    output_file = "compiled.txt"
    directory = "./asm"

    file_paths = []

    # Walk through the directory and its subdirectories
    for root, _, files in os.walk(directory):
        for file in files:
            # Construct the full file path
            full_path = os.path.join(root, file)
            file_paths.append(full_path)

    # remove Makefiles
    to_remove = []
    for i in range(len(file_paths)):
        if (file_paths[i].__contains__("Makefile")):
            to_remove.append(file_paths[i])

    for e in to_remove:
        file_paths.remove(e)

    ranges = concatenate_files(output_file, *file_paths)
    expand_macros(output_file)
    compile(output_file)
