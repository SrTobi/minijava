/**
 * @file token_string.hpp
 *
 * @brief
 *     helper functions for creating vectors of std::unique_ptr
 *
 */

#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "meta/meta.hpp"


namespace testaux
{
	/**
	 * @brief
	 *     Creates a vector out of `unique_ptr`s whose underlying types share
	 *     a base class `T`.
	 *
	 * @param args
	 *     `unique_ptr`s to move into the new vector
	 *
	 * @tparam
	 *     common type of all underlying types
	 *
	 * @tparam ArgTs
	 *     types of the `unique_ptr`s to move into the new vector
	 *
	 * @returns
	 *     vector of the given `unique_ptr`s
	 *
	 */
	template <typename T, typename... ArgTs>
	std::enable_if_t<
		minijava::meta::conjunction<std::is_convertible<ArgTs, std::unique_ptr<T>>...>{},
		std::vector<std::unique_ptr<T>>
	>
	make_unique_ptr_vector(ArgTs... args);
}

#define TESTAUX_INCLUDED_FROM_UNIQUE_PTR_VECTOR_HPP
#include "testaux/unique_ptr_vector.tpp"
#undef TESTAUX_INCLUDED_FROM_UNIQUE_PTR_VECTOR_HPP
