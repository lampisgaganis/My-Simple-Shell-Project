## My Simple Shell Project (mysh)

**My Simple Shell Project** is a C++ program aiming to emulate functionality of widely used Unix shells such as _tcsh_ and _bash_. A Unix shell is a command-line interpreter or shell that provides a command line user interface for Unix-like operating systems.

Features of My Simple Shell include:

* I/O Redirection

  * IO redirection is supported for standard input (<), standard output (> and >>), allowing users to control the flow of data. In this way, output from one program can be used as input for another and so forth.

* Command Pipelines:

  * Users can execute command pipelines using the pipe ('|') symbol, enabling the chaining of multiple commands.

* Wildcard Expansion:

  * Wildcard characters ('*' and '?') are supported for filename expansion using the glob function.

* Background execution:

  * Background execution is supported, allowing the user to run multiple commands concurrently in the background.

* Signal Handling:

  * The program handles SIGTSTP and SIGINT signals, providing a basic signal handler.

This project was part of the System Programming course, exploring advanced concepts such as command parsing and execution, system calls, parent and child processes, concurrent processes, inter process communication and many more.

COMPILE: ```make```

COMMAND TO RUN: ```./mysh```

Recommended commands to test functionality:

```wget https://ftp.ebi.ac.uk/pub/databases/genenames/hgnc/tsv/locus_types/gene_with_protein_product.txt

cat gene_with_protein product.txt

cut -f2 gene_with_protein product.txt > gene_with_protein_product_names_only_vl.txt

cat gene_with_protein_product_names_only_vl.txt

cat gene_with_protein_product.txt | cut -f2 > gene_with_protein_product_names_only_v2.txt

diff gene_with_protein_product_names_only_v2.txt gene_with_protein_product_names_only_vl.txt

shuf gene_with_protein_product_names_only_vl.txt

shuf gene_with_protein_product_names_only_vl.txt > gene_with_protein_product_names_only_vl_shuffled.txt

sort < gene_with_protein_product_names_only_vl_shuffled.txt > gene_with_protein_product_names_only_vl_sorted.txt

head -100 gene_with_protein_product_names_only_vl.txt > h100.txt

tail -50 gene_with_protein_product_names_only_vl.txt > t50.txt

cat h100.txt t50.txt > ht150_v1.txt

wc -1 ht150_v1.txt

cat h100.txt > ht150_v2.txt

cat t50.txt >> ht150_v2.txt

diff ht150_v1.txt ht150_v2.txt

ls gene*```
