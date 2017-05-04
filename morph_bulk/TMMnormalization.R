#!/usr/bin/Rscript
#------------------------------------------------
# R script for TMM normalization of RNA-Seq data
# Input: 
#   Tab delimited gene expression matrix
#   rownames are genes, column names conditions
#   values are read counts (e.g. from HTSeq)
# Output:
#   Normalized expression matrix, using TMM
#   normalization (Robinson & Oshlack 2010)
#------------------------------------------------

library(edgeR)

commandline_args = commandArgs(trailingOnly = T)
expression_matrix = commandline_args[1]
outputfile = commandline_args[2]

matrix = read.table(expression_matrix, header = T, row.names = 1)
y = calcNormFactors(matrix)
normalized  = t(t(as.matrix(matrix)) * y)

write.table(normalized, file = outputfile, sep = "\t", quote = F, col.names = NA, row.names = T)

