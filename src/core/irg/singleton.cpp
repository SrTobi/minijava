#include "irg/singleton.hpp"

#include <atomic>
#include <cassert>
#include <stdexcept>
#include <utility>

#include "firm.hpp"

#include "irg/type_builder.hpp"
#include "irg/method_builder.hpp"


namespace minijava
{

	firm_global_state::firm_global_state()
	{
		static std::atomic_flag initialized = ATOMIC_FLAG_INIT;
		if (initialized.test_and_set()) {
			throw std::logic_error{
					"libfirm was already initialized and is not re-entrant"
			};
		}
		firm::ir_init();
		firm::set_optimize(0);
		const auto mode_p = firm::new_reference_mode("P64", firm::irma_twos_complement, 64, 64);
		firm::set_modeP(mode_p);
	}

	firm_global_state::firm_global_state(firm_global_state&& other)
		: _firm_owner{std::exchange(other._firm_owner, false)}
	{
	}

	firm_global_state::~firm_global_state()
	{
		if (_firm_owner) {
			firm::ir_finish();
		}
	}

	firm_global_state::operator bool() const noexcept
	{
		return _firm_owner;
	}

}  // namespace minijava
