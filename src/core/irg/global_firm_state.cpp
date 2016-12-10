#include "irg/global_firm_state.hpp"

#include <atomic>
#include <cassert>
#include <stdexcept>
#include <utility>

#include "firm.hpp"

#include "irg/type_builder.hpp"
#include "irg/method_builder.hpp"


namespace minijava
{
	namespace detail {
		struct global_firm_state_instance
		{
		};
	}

	namespace
	{
		void uninitialize_firm(detail::global_firm_state_instance* firm)
		{
			assert(firm);
			firm::ir_finish();
		}
	}

	global_firm_state initialize_firm()
	{
		static std::atomic_flag initialized = ATOMIC_FLAG_INIT;
		if(initialized.test_and_set()) {
			throw std::logic_error{
				"libfirm was already initialized and is not re-entrant"
			};
		}
		firm::ir_init();
		firm::set_optimize(0);
		const auto mode_p = firm::new_reference_mode("P64", firm::irma_twos_complement, 64, 64);
		firm::set_modeP(mode_p);

		//firm::free_ir_prog();

		static detail::global_firm_state_instance instance{};
		return {&instance, &uninitialize_firm};
	}

}  // namespace minijava
