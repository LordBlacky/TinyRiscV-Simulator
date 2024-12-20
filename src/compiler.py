import os
import re

instructions_dict = {
    "ADD": 0,
    "SUB": 1,
    "AND": 2,
    "OR": 3,
    "XOR": 4,
    "SLT": 5,
    "SLTU": 6,
    "SRA": 7,
    "SRL": 8,
    "SLL": 9,
    "MUL": 10,
    "SLLI": 11,
    "ADDI": 12,
    "ANDI": 13,
    "ORI": 14,
    "XORI": 15,
    "SLTI": 16,
    "SLTIU": 17,
    "SRAI": 18,
    "SRLI": 19,
    "LUI": 20,
    "AUIPC": 21,
    "LW": 22,
    "SW": 23,
    "BEQ": 24,
    "BNE": 25,
    "BLT": 26,
    "BGE": 27,
    "BLTU": 28,
    "BGEU": 29,
    "JAL": 30,
    "JALR": 31,
    "FLAG": 32,
    "EMPTY": 33
}


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


def compile(file):
    """
    removes comments and replace labels with actuall immediates
    also adds a JAL instruction to the lable _start
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

    print(lines)
    # iterate through lines and replace everything with ids
    for i in range(count):
        parts = re.split(r",? |\(", lines[i].strip())
        print(parts)
        # parts = lines[i].split()
        if (parts[0] == ''):
            continue

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

    print(lines)


def get_arg_id(arg: str, labels: dict, current_line):
    # check if its a register
    r = re.match(r"x(\d\d?)", arg)
    print(r)
    if (r):
        return int(r.group(1))

    # check if its a immediate
    if (re.match(r"\d+", arg)):
        return int(arg)

    # check if its a label
    if (labels.__contains__(arg)):
        return (labels[arg] - current_line) * 4

    print(f"Error: '{arg}' can't be resolved")


def get_inst_id(arg: str):
    arg = arg.upper()
    return instructions_dict[arg]


if __name__ == "__main__":
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

    compile(output_file)
