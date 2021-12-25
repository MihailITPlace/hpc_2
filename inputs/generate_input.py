import sys
import numpy as np

matrix_size = int(sys.argv[1])
matrix = np.random.random((matrix_size, matrix_size))
vector = np.random.random((1, matrix_size))

np.savetxt(f'matrix-{matrix_size}', matrix, fmt='%lf')
np.savetxt(f'vector-{matrix_size}', matrix, fmt='%lf')

