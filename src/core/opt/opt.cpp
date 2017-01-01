#include "opt/opt.hpp"


namespace minijava
{

	std::vector<std::unique_ptr<minijava::opt::optimization>> optimizations;

	void optimize(firm_ir& ir)
	{
		auto changed = false;
		do
		{
			for (auto& opt : optimizations) {
				changed |= opt->optimize(ir);
			}
		} while (changed);
	}

	void register_optimization(std::unique_ptr<minijava::opt::optimization> opt)
	{
		optimizations.push_back(std::move(opt));
	}

	void register_all_optimizations()
	{ }

}  // namespace minijava
