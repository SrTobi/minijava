#include "opt/opt.hpp"

#include <cassert>

#include "irg/irg.hpp"
#include "opt/lowering.hpp"


namespace minijava
{

	void optimize(firm_ir& ir)
	{
		assert(ir);
		const auto guard = make_irp_guard(*ir->second, ir->first);
		// TODO: Implement (in particular, constant propagation)
		opt::lower();
	}

}  // namespace minijava
