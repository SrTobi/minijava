/**
 * @file tailrec.hpp
 *
 * @brief
 *     tail recursion optimization.
 *
 */

#pragma once

#include "opt/opt.hpp"

namespace minijava
{
	namespace opt
	{
		/**
		 * @brief
		 *     Simple tail recursion optimization.
		 *
		 *     Searches for functions, which leave with a recursiv call and tries to replace
		 *     them with a jump back to the head.
		 *     I.e.
		 *
		 *     int foo(int x) {
		 *         if (x > 0)
		 *             return foo(x - 1);
		 *         return x;
		 *     }
		 *
		 *     could be
		 *
		 *     inf foo(int x) {
		 *     head:
		 *         if (x > 0) {
		 *             x = x - 1;
		 *             goto head;
		 *         }
		 *         return x;
		 *     }
		 */
		class tailrec: public optimization
		{
		private:
			/**
			 * @brief
			 *     Set to true, if we found and replaced a tail recursion.
			 */
			bool _changed{false};

		public:
			/**
			 * @brief
			 *     Searches in all irg's for tail recursion and tries to replace them
			 * @return
			 */
			virtual bool optimize(firm_ir &) override;
		};
	}
}
