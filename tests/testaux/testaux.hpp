/**
 * @file testaux.hpp
 *
 * @brief
 *         This header file provides a grab bag of auxiliary features
 *         that were considered useful for writing unit tests.
 *
 */

#pragma once

#include <utility>

namespace testaux
{

	template <typename T>
	T maybe_not_that_useful(T&& arg) {
		return std::forward<T>(arg);
	}

}

// namespace testaux
