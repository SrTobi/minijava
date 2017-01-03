#include "opt/opt.hpp"
#include "opt/algebraic_simplifier.hpp"
#include <iostream>

namespace minijava
{

	std::vector<std::unique_ptr<minijava::opt::optimization>> optimizations;

	void optimize(firm_ir& ir)
	{
		const auto guard = make_irp_guard(*ir->second, ir->first);
		bool changed;
		unsigned int count = 0;
		unsigned int max_count = 99;
		do
		{
			changed = false;
			for (auto& opt : optimizations) {
				changed |= opt->optimize(ir);
			}
		} while (changed && count++ < max_count);
	}

	void register_optimization(std::unique_ptr<minijava::opt::optimization> opt)
	{
		optimizations.push_back(std::move(opt));
	}

	void register_all_optimizations()
	{
		register_optimization(std::make_unique<opt::algebraic_simplifier>());
	}

}  // namespace minijava
