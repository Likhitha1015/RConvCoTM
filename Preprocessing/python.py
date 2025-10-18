input_file = "MNIST_clauses_7_1.txt"
output_file = "memory_data_mnist_7_clauses.h"

with open(input_file, "r") as f_in:
    lines = [line.strip() for line in f_in if line.strip()]

with open(output_file, "w") as f_out:
    f_out.write("#ifndef MEMORY_DATA_H\n")
    f_out.write("#define MEMORY_DATA_H\n\n")
    f_out.write("static const unsigned int memory_data[] = {\n")

    for line_idx, line in enumerate(lines):
        # Split line into 32-bit chunks
        for i in range(0, len(line), 32):
            chunk = line[i:i+32]
            word = int(chunk, 2)  # Convert binary string to int
            f_out.write(f"    0x{word:08x},  // line {line_idx} word {i//32}\n")

    f_out.write("};\n\n")
    f_out.write("static const int memory_data_size = sizeof(memory_data) / sizeof(memory_data[0]);\n\n")
    f_out.write("#endif // MEMORY_DATA_H\n")

print(f"✅ Generated {output_file} with {len(lines)} lines, each split into {len(line)//32} words")
