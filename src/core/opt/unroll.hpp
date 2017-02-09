/**
 * @file unroll.hpp
 *
 * @brief
 *     loop unroll optimization.
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
		 *     Loop unroll optimization.
		 *
		 *     Searches in every irg for loops, with our desired structure and tries to unroll
		 *     them.
		 *     I.e.
		 *
		 *     int x = 0;
		 *     int y = 0;
		 *     while (x < 10) {
		 *         y = y + x;
		 *         x = x + 1;
		 *     }
		 *     System.out.println(y);
		 *
		 *     could be
		 */
		class unroll: public optimization
		{
		private:
			/**
			 * @brief
			 *     Set to true, if we could unroll any loop.
			 */
			bool _changed{false};

		public:
			/**
			 * @brief
			 *     Searches in all irg's for a loop with our desired structure to unroll
			 * @return
			 */
			virtual bool optimize(firm_ir &) override;
		};
	}
}