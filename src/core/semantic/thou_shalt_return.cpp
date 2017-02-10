#include "semantic/thou_shalt_return.hpp"

#include <string>

#include "semantic/semantic_error.hpp"


namespace minijava
{

	namespace sem
	{

		namespace /* anonymous */
		{

			struct can_complete_normally_visitor final : ast::visitor
			{

				bool result{true};

				using ast::visitor::visit;

				void visit(const ast::block& blck) override
				{
					for (const auto& bstm : blck.body()) {
						bstm->accept(*this);
						if (!result) {
							break;
						}
					}
				}

				void visit(const ast::return_statement& /* rtst */) override
				{
					result = false;
				}

				void visit(const ast::if_statement& ifst) override
				{
					if (ifst.else_statement() == nullptr) {
						return;
					}
					auto then_can = can_complete_normally_visitor{};
					ifst.then_statement().accept(then_can);
					if (then_can.result) {
						return;
					}
					auto else_can = can_complete_normally_visitor{};
					ifst.else_statement()->accept(else_can);
					if (else_can.result) {
						return;
					}
					result = false;
				}

			};

			bool can_complete_normally(const ast::block_statement& bstm)
			{
				auto ccnv = can_complete_normally_visitor{};
				bstm.accept(ccnv);
				return ccnv.result;
			}

			void check_method(const ast::instance_method& mthd)
			{
				using namespace std::string_literals;
				if (can_complete_normally(mthd.body())) {
					throw semantic_error{
							"Control-flow possibly reaches end of non-void function "s
									+ "'"s + mthd.name().c_str() + "'",
							mthd.position()
					};
				}
			}

		}  // namespace /* anonymous */

		void check_return_paths(const ast::program& prog,
								const type_attributes& types)
		{
			for (const auto& claz : prog.classes()) {
				for (const auto& mthd : claz->instance_methods()) {
					if (!types.at(*mthd).info.is_void()) {
						check_method(*mthd);
					}
				}
			}
		}

	}  // namespace sem

}  // namespace minijava
