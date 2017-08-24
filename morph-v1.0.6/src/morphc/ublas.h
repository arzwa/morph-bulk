// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#define BOOST_UBLAS_NDEBUG  // comment this line for massive ublas assertions (slows down debug a lot. You should do it at least once after changing matrix/vector computations though, with a very small joblist + 1 dataset + 1clusterings)
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace MORPHC {

typedef boost::numeric::ublas::matrix<double> matrix;
typedef boost::numeric::ublas::matrix_indirect<matrix> matrix_indirect;
typedef matrix::size_type size_type;
//typedef boost::numeric::ublas::matrix<double>::array_type array_type;
typedef boost::numeric::ublas::indirect_array<> indirect_array;
typedef MORPHC::indirect_array::array_type array;

}
