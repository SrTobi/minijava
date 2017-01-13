#include "irg/global_firm_state.hpp"

#include <atomic>
#include <cassert>
#include <stdexcept>
#include <utility>

#include "firm.hpp"

#include "irg/type_builder.hpp"
#include "irg/method_builder.hpp"


namespace /* anonymous */
{

	// Counts the currently live Firm program IRGs.  This could be a `static`
	// data member of `global_firm_state` but then we would have to expose it
	// in the header file and we need to initialize it here anyway.
	std::atomic_int _counter{0};

}

namespace minijava
{

	global_firm_state::global_firm_state()
	{
		static std::atomic_flag initialized = ATOMIC_FLAG_INIT;
		if (initialized.test_and_set()) {
			throw std::logic_error{"libfirm was already initialized before"};
		}
		firm::ir_init();
		firm::set_optimize(0);
		const auto mode_p = firm::new_reference_mode("P64", firm::irma_twos_complement, 64, 64);
		firm::set_modeP(mode_p);
		_irp = firm::get_irp();
		_counter.store(1);
	}

	global_firm_state::~global_firm_state()
	{
		const auto n = _counter.exchange(0);
		assert(n == 1);  (void) n;
		if (true) {
			// TODO: Is this necessary or will `firm::ir_finish` clean up anyway?
			firm::set_irp(_irp);
			firm::free_ir_prog();
		}
		firm::ir_finish();
	}

	firm::ir_prog* global_firm_state::new_ir_prog(const char*const name)
	{
		assert(name != nullptr);
		const auto p = firm::new_ir_prog(name);
		assert(p != nullptr);
		firm::set_irp(p);
		const auto n = _counter.fetch_add(1);
		assert(n > 0);  (void) n;
		return p;
	}

	void global_firm_state::free_ir_prog(firm::ir_prog*const p)
	{
		assert(p != nullptr);
		assert(p != _irp);
		firm::set_irp(p);
		firm::free_ir_prog();
		firm::set_irp(_irp);
		const auto n = _counter.fetch_sub(1);
		assert(n > 1);  (void) n;
	}

	firm::ir_prog* global_firm_state::get_default_irp() const noexcept
	{
		return _irp;
	}

	int global_firm_state::program_count() noexcept
	{
		return _counter.load();
	}

}  // namespace minijava
