#ifndef TESTAUX_INCLUDED_FROM_ASTGEN_HPP
#error "Never `#include <testaux/astgen.tpp>` directly; `#include <testaux/astgen.hpp>` instead."
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iterator>
#include <random>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "exceptions.hpp"
#include "symbol/symbol.hpp"

#include "testaux/random_tokens.hpp"


namespace testaux
{

	namespace detail
	{

		template <typename T, typename DelT>
		std::unique_ptr<T, DelT> make_unique_ptr(T* ptr, DelT del)
		{
			return {ptr, del};
		}


		class yet_another_type_type
		{
		public:

			yet_another_type_type()
				: yet_another_type_type {minijava::ast::primitive_type::type_void}
			{
			}

			yet_another_type_type(const minijava::ast::type& type)
				: _type_name{type.name()}
				, _rank{type.rank()}
			{
			}

			yet_another_type_type(minijava::ast::primitive_type primitive, const std::size_t rank = 0)
				: _type_name{primitive}
				, _rank{rank}
			{
			}

			yet_another_type_type(const minijava::symbol classname, const std::size_t rank = 0)
				: _type_name{classname}
				, _rank{rank}
			{
			}

			const minijava::ast::type_name& name() const noexcept
			{
				return _type_name;
			}

			std::size_t rank() const noexcept
			{
				return _rank;
			}

			yet_another_type_type array() const
			{
				auto copy = *this;
				copy._rank += 1;
				return copy;
			}

		private:

			minijava::ast::type_name _type_name;

			std::size_t _rank;

		};  // class yet_another_type_type


		inline bool is_void(const yet_another_type_type& type)
		{
			if (const auto p = boost::get<minijava::ast::primitive_type>(&type.name())) {
				return (*p == minijava::ast::primitive_type::type_void);
			}
			return false;
		}

		inline bool is_integer(const yet_another_type_type& type)
		{
			if (const auto p = boost::get<minijava::ast::primitive_type>(&type.name())) {
				return (*p == minijava::ast::primitive_type::type_int);
			}
			return false;
		}

		inline bool is_boolean(const yet_another_type_type& type)
		{
			if (const auto p = boost::get<minijava::ast::primitive_type>(&type.name())) {
				return (*p == minijava::ast::primitive_type::type_boolean);
			}
			return false;
		}

		inline bool is_class(const yet_another_type_type& type)
		{
			return boost::get<minijava::symbol>(&type.name());
		}

		inline bool is_primitive(const yet_another_type_type& type)
		{
			return !is_class(type);
		}

		inline bool is_array(const yet_another_type_type& type)
		{
			return (type.rank() > 0);
		}

		inline bool equal(const yet_another_type_type& lhs, const yet_another_type_type& rhs)
		{
			return (lhs.rank() == rhs.rank()) && (lhs.name() == rhs.name());
		}


		struct field_signature
		{
			minijava::symbol name{};
			yet_another_type_type type{};
		};


		struct method_signature
		{
			minijava::symbol name{};
			yet_another_type_type return_type{};
			std::vector<yet_another_type_type> parameter_types{};
		};


		template <typename EngineT, typename PoolT>
		class ast_generator
		{
		public:

			ast_generator(
				EngineT& engine,
				PoolT& pool,
				minijava::ast_factory& factory,
				const std::size_t limit
			)
				: _engine{engine}
				, _pool{pool}
				, _factory{factory}
				, _nest_limit{limit}
				, _nest_depth{1}
			{
				assert(limit > 0);
			}

			std::unique_ptr<minijava::ast::program>
			operator()()
			{
				_gen_class_names();
				_gen_class_fields();
				_gen_class_methods();
				const auto mainclass = _random_choice(_class_names);
				auto classes = std::vector<std::unique_ptr<minijava::ast::class_declaration>>{};
				std::transform(
					std::begin(_class_names), std::end(_class_names),
					std::back_inserter(classes),
					[self = this, mainclass](auto&& name){
						return self->_make_class(name, name == mainclass);
					}
				);
				auto program = _factory.make<minijava::ast::program>()(std::move(classes));
				_class_names.clear();
				_class_fields.clear();
				_class_methods.clear();
				return program;
			}

		private:

			EngineT& _engine;
			PoolT& _pool;
			minijava::ast_factory& _factory;

			std::size_t _nest_limit;
			std::size_t _nest_depth;

			std::vector<minijava::symbol> _class_names{};
			std::unordered_map<minijava::symbol, std::vector<field_signature>> _class_fields{};
			std::unordered_map<minijava::symbol, std::vector<method_signature>> _class_methods{};
			std::vector<std::vector<const minijava::ast::var_decl*>> _scopes{};
			minijava::symbol _current_class{};
			yet_another_type_type _current_return_type{};

			static yet_another_type_type _scalar_int()
			{
				return {minijava::ast::primitive_type::type_int, 0};
			}

			static yet_another_type_type _scalar_boolean()
			{
				return {minijava::ast::primitive_type::type_boolean, 0};
			}

			double _dist_param() const noexcept
			{
				assert((_nest_depth > 0) && (_nest_depth < _nest_limit));
				const auto depth = static_cast<double>(_nest_depth);
				const auto limit = static_cast<double>(_nest_limit);
				const auto p = 1.0 - std::sqrt(depth / limit);
				assert((p > 0.0) && (p < 1.0));
				return p;
			}

			auto _enter_nested()
			{
				const auto del = [](ast_generator* p){ p->_nest_depth -= 1; };
				_nest_depth += 1;
				return std::unique_ptr<ast_generator, decltype(del)>{this, del};
			}

			bool _nest_deeper_eh()
			{
				if (_nest_depth >= _nest_limit) {
					return false;
				} else {
					auto dist = std::bernoulli_distribution{_dist_param()};
					return dist(_engine);
				}
			}

			std::size_t _fan_out_n(const double factor = 1.0)
			{
				if (_nest_depth >= _nest_limit) {
					return 0;
				} else {
					const auto p = (1.0 - _dist_param()) / factor;
					auto dist = std::geometric_distribution<std::size_t>{p};
					return dist(_engine);
				}
			}

			auto _do_enter_scope()
			{
				const auto del = [](auto* p){ p->_scopes.pop_back(); };
				_scopes.emplace_back();
				return std::unique_ptr<ast_generator, decltype(del)>{this, del};
			}

			auto _set_return_type(const yet_another_type_type& type)
			{
				const auto del = [](ast_generator* p){
					p->_current_return_type = yet_another_type_type{};
				};
				_current_return_type = type;
				return std::unique_ptr<ast_generator, decltype(del)>{this, del};
			}

			bool _can_declare(const minijava::symbol name)
			{
				assert(_scopes.size() > 1);
				const auto first = std::rbegin(_scopes);
				const auto last = std::prev(std::rend(_scopes));
				for (auto it = first; it != last; ++it) {
					const auto pos = std::find_if(
						std::begin(*it), std::end(*it),
						[name](auto&& vdcl){ return vdcl->name() == name; }
					);
					if (pos != std::end(*it)) {
						return false;
					}
				}
				return true;
			}

			void _gen_class_names()
			{
				_class_names = _make_unique_names();
				if (_class_names.empty()) {
					_class_names.push_back(_pool.normalize(get_random_identifier(_engine)));
				}
			}

			void _gen_class_fields()
			{
				_class_fields.clear();
				for (auto&& classname : _class_names) {
					const auto names = _make_unique_names();
					auto fields = std::vector<field_signature>{};
					std::transform(
						std::begin(names), std::end(names),
						std::back_inserter(fields),
						[self = this](auto&& n){
							return field_signature{n, self->_random_type(false)};
						}
					);
					_class_fields[classname] = std::move(fields);
				}
			}

			void _gen_class_methods()
			{
				_class_methods.clear();
				for (auto&& classname : _class_names) {
					const auto names = _make_unique_names();
					auto signatures = std::vector<method_signature>{};
					std::transform(
						std::begin(names), std::end(names),
						std::back_inserter(signatures),
						[self = this](auto&& name){
							return self->_random_signature(name);
						}
					);
					_class_methods[classname] = std::move(signatures);
				}
			}

			std::unique_ptr<minijava::ast::class_declaration>
			_make_class(const minijava::symbol name, const bool with_main)
			{
				const auto g = _enter_nested();
				auto fields = std::vector<std::unique_ptr<minijava::ast::var_decl>>{};
				auto instance_methods = std::vector<std::unique_ptr<minijava::ast::instance_method>>{};
				auto main_methods = std::vector<std::unique_ptr<minijava::ast::main_method>>{};
				for (const auto& sig : _class_fields[name]) {
					auto field = _factory.make<minijava::ast::var_decl>()
						(_make_type(sig.type), sig.name);
					fields.push_back(std::move(field));
				}
				{
					const auto class_guard = make_unique_ptr(
						this,
						[](ast_generator* p){
							p->_current_class = minijava::symbol{};
						}
					);
					_current_class = name;
					const auto scope_guard = _do_enter_scope();
					std::for_each(
						std::begin(fields), std::end(fields),
						[self = this](auto&& vdcl){
							self->_scopes.back().push_back(vdcl.get());
						}
					);
					std::transform(
						std::begin(_class_methods[name]), std::end(_class_methods[name]),
						std::back_inserter(instance_methods),
						[self = this, name](auto&& sig){
							return self->_make_instance_method(sig);
						}
					);
				}
				if (with_main) {
					main_methods.push_back(_make_main_method());
				}
				return _factory.make<minijava::ast::class_declaration>()(
					name,
					std::move(fields),
					std::move(instance_methods),
					std::move(main_methods)
				);
			}

			std::unique_ptr<minijava::ast::instance_method>
			_make_instance_method(method_signature signature)
			{
				const auto g = _enter_nested();
				const auto ret_guard = _set_return_type(signature.return_type);
				const auto scope_guard = _do_enter_scope();
				auto params = std::vector<std::unique_ptr<minijava::ast::var_decl>>{};
				for (const auto& yatt : signature.parameter_types) {
					auto name = minijava::symbol{};
					do {
						name = _pool.normalize(get_random_identifier(_engine, 0.5));
					} while (!_can_declare(name));
					auto decl = _factory.make<minijava::ast::var_decl>()
						(_make_type(yatt), name);
					_scopes.back().push_back(decl.get());
					params.push_back(std::move(decl));
				}
				const auto inner_scope_guard = _do_enter_scope();
				auto stmts = std::vector<std::unique_ptr<minijava::ast::block_statement>>{};
				std::generate_n(
					std::back_inserter(stmts),
					_fan_out_n(),
					[self = this](){ return self->_make_block_statement(); }
				);
				if (!is_void(signature.return_type)) {
					stmts.push_back(_make_return_statement());
				}
				return _factory.make<minijava::ast::instance_method>()(
					signature.name,
					_make_type(signature.return_type),
					std::move(params),
					_factory.make<minijava::ast::block>()(std::move(stmts))
				);
			}

			std::unique_ptr<minijava::ast::main_method>
			_make_main_method()
			{
				const auto g = _enter_nested();
				const auto scope_guard = _do_enter_scope();
				return _factory.make<minijava::ast::main_method>()(
					_pool.normalize("main"),
					_pool.normalize("args"),
					_make_block()
				);
			}

			method_signature _random_signature(minijava::symbol name)
			{
				auto rettype = _random_type(true);
				auto params = std::vector<yet_another_type_type>{};
				std::generate_n(
					std::back_inserter(params),
					_fan_out_n(),
					[self = this](){ return self->_random_type(false); }
				);
				return {std::move(name), std::move(rettype), std::move(params)};
			}

			std::vector<std::unique_ptr<minijava::ast::var_decl>>
			_make_var_decls()
			{
				const auto names = _make_unique_names();
				auto decls = std::vector<std::unique_ptr<minijava::ast::var_decl>>{};
				std::transform(
					std::begin(names), std::end(names),
					std::back_inserter(decls),
					[self = this](auto&& name){
						return self->_factory.template make<minijava::ast::var_decl>()
							(self->_make_type(false), name);
					}
				);
				return decls;
			}

			std::unique_ptr<minijava::ast::block_statement>
			_make_block_statement()
			{
				const auto g = _enter_nested();
				auto dist = std::bernoulli_distribution{0.2};
				if (dist(_engine)) {
					return _make_local_variable_statement();
				} else {
					return _make_statement();
				}
			}

			std::unique_ptr<minijava::ast::local_variable_statement>
			_make_local_variable_statement()
			{
				const auto g = _enter_nested();
				auto type = _make_type(false);
				auto decl = std::unique_ptr<minijava::ast::var_decl>{};
				for (auto p = 0.5; true; p /= 2.0) {
					const auto name = _pool.normalize(get_random_identifier(_engine, p));
					if (_can_declare(name)) {
						decl = _factory.make<minijava::ast::var_decl>()
							(std::move(type), name);
						break;
					}
				}
				auto dist = std::bernoulli_distribution{};
				auto value = std::unique_ptr<minijava::ast::expression>{};
				_scopes.back().push_back(decl.get());
				if (dist(_engine)) {
					value = _make_expression(decl->var_type());
				}
				return _factory.make<minijava::ast::local_variable_statement>()
					(std::move(decl), std::move(value));
			}

			std::unique_ptr<minijava::ast::statement>
			_make_statement()
			{
				const auto g = _enter_nested();
				auto dist = std::uniform_int_distribution<>{1, 6};
				switch (dist(_engine)) {
				case 1:
					return _make_block();
				case 2:
					return _factory.make<minijava::ast::empty_statement>()();
				case 3:
					return _make_expression_statement();
				case 4:
					return _make_if_statement();
				case 5:
					return _make_while_statement();
				case 6:
					return _make_return_statement();
				}
				MINIJAVA_NOT_REACHED();
			}

			std::unique_ptr<minijava::ast::block>
			_make_block()
			{
				const auto g = _enter_nested();
				const auto scope_guard = _do_enter_scope();
				auto blkstmts = std::vector<std::unique_ptr<minijava::ast::block_statement>>{};
				std::generate_n(
					std::back_inserter(blkstmts),
					_fan_out_n(1.5),
					[self = this](){ return self->_make_block_statement(); }
				);
				return _factory.make<minijava::ast::block>()(std::move(blkstmts));
			}

			std::unique_ptr<minijava::ast::expression_statement>
			_make_expression_statement()
			{
				const auto g = _enter_nested();
				auto inner = std::unique_ptr<minijava::ast::expression>{};
				do {
					const auto yatt = _random_type(true);
					inner = _make_expression(yatt);
				} while (!inner);
				return _factory.make<minijava::ast::expression_statement>()
					(std::move(inner));
			}

			std::unique_ptr<minijava::ast::if_statement>
			_make_if_statement()
			{
				const auto g = _enter_nested();
				auto cond = _make_boolean_expression();
				auto then = _make_statement();
				auto othr = std::unique_ptr<minijava::ast::statement>{};
				if (_nest_deeper_eh()) {
					othr = _make_statement();
				}
				return _factory.make<minijava::ast::if_statement>()
					(std::move(cond), std::move(then), std::move(othr));
			}

			std::unique_ptr<minijava::ast::while_statement>
			_make_while_statement()
			{
				const auto g = _enter_nested();
				auto cond = _make_boolean_expression();
				auto body = _make_statement();
				return _factory.make<minijava::ast::while_statement>()
					(std::move(cond), std::move(body));
			}

			std::unique_ptr<minijava::ast::return_statement>
			_make_return_statement()
			{
				const auto g = _enter_nested();
				auto value = std::unique_ptr<minijava::ast::expression>{};
				if (!is_void(_current_return_type)) {
					value = _make_expression(_current_return_type);
				}
				return _factory.make<minijava::ast::return_statement>()
					(std::move(value));
			}

			// If `is_void(type)` and there is no method of type `void` in the
			// program, an empty pointer will be `return`ed.
			std::unique_ptr<minijava::ast::expression>
			_make_expression_impl(const yet_another_type_type& type, const int quality)
			{
				const auto g = _enter_nested();
				if (is_void(type)) {
					return _maybe_make_method_invocation(type);
				}
				auto dist = std::bernoulli_distribution{};
				while (true) {
					if (dist(_engine)) {
						if (auto node = _maybe_make_variable_access(type)) {
							return node;
						}
					}
					if ((quality >= 2) || _nest_deeper_eh()) {
						const auto most = [&type, quality](){
							if (quality >= 2) {
								return 1;
							} else if (!is_primitive(type) || is_array(type)) {
								return 3;
							} else {
								return 9;
							}
						}();
						auto dist = std::uniform_int_distribution<>{1, most};
						switch (dist(_engine)) {
						case 1:
							return _factory.make<minijava::ast::array_access>()(
								_make_target(type.array()),
								_make_integral_expression()
							);
						case 2:
							return _make_assignment(type);
						case 3:
							if (auto node = _maybe_make_method_invocation(type)) {
								return std::unique_ptr<minijava::ast::expression>{node.release()};  // Clang?!
							}
							break;
						case 4: case 5: case 6: case 7:
							return _make_binary_expression(type);
						case 8: case 9:
							return _make_unary_expression(type);
						default:
							MINIJAVA_NOT_REACHED();
						}
					}
					if (dist(_engine)) {
						if (auto node = _maybe_make_this_ref(type)) {
							return std::unique_ptr<minijava::ast::expression>{node.release()};  // Clang?!
						}
						if (is_class(type) && (type.rank() == 0)) {
							return _factory.make<minijava::ast::object_instantiation>()
								(boost::get<minijava::symbol>(type.name()));
						}
						if (is_array(type)) {
							auto extent = _make_integral_expression();
							return _factory.make<minijava::ast::array_instantiation>()
								(_make_type(type), std::move(extent));
						}
					}
					if (quality < 1) {
						return _make_constant(type);
					}
				}
			}

			std::unique_ptr<minijava::ast::expression>
			_make_integral_expression()
			{
				return _make_expression(_scalar_int());
			}

			std::unique_ptr<minijava::ast::expression>
			_make_boolean_expression()
			{
				return _make_expression(_scalar_boolean());
			}

			std::unique_ptr<minijava::ast::expression>
			_make_expression(const yet_another_type_type& type)
			{
				return _make_expression_impl(type, 0);
			}

			std::unique_ptr<minijava::ast::expression>
			_make_target(const yet_another_type_type& type)
			{
				assert(!is_void(type));
				return _make_expression_impl(type, 1);
			}

			std::unique_ptr<minijava::ast::expression>
			_make_destination(const yet_another_type_type& type)
			{
				assert(!is_void(type));
				return _make_expression_impl(type, 2);
			}

			std::unique_ptr<minijava::ast::binary_expression>
			_make_assignment(const yet_another_type_type& type)
			{
				assert(!is_void(type));
				const auto g = _enter_nested();
				return _factory.make<minijava::ast::binary_expression>()(
					minijava::ast::binary_operation_type::assign,
					_make_destination(type),
					_make_expression(type)
				);
			}

			std::unique_ptr<minijava::ast::binary_expression>
			_make_binary_expression(const yet_another_type_type& type)
			{
				assert(!is_void(type));
				assert(is_primitive(type));
				assert(!is_array(type));
				assert(!is_void(type) && is_primitive(type) && !is_array(type));
				const auto g = _enter_nested();
				if (is_integer(type)) {
					const auto operation = _random_choice_from(
						minijava::ast::binary_operation_type::plus,
						minijava::ast::binary_operation_type::minus,
						minijava::ast::binary_operation_type::multiply,
						minijava::ast::binary_operation_type::divide,
						minijava::ast::binary_operation_type::modulo
					);
					return _factory.make<minijava::ast::binary_expression>()(
						operation,
						 _make_integral_expression(),
						 _make_integral_expression()
					);
				}
				if (is_boolean(type)) {
					auto operand_type = yet_another_type_type{};
					auto operation = minijava::ast::binary_operation_type{};
					auto dist = std::uniform_int_distribution<>{1, 3};
					switch (dist(_engine)) {
					case 1:
						operand_type = _random_type(false);
						operation = _random_choice_from(
							minijava::ast::binary_operation_type::equal,
							minijava::ast::binary_operation_type::not_equal
						);
						break;
					case 2:
						operand_type = _scalar_boolean();
						operation = _random_choice_from(
							minijava::ast::binary_operation_type::logical_or,
							minijava::ast::binary_operation_type::logical_and
						);
						break;
					case 3:
						operand_type = _scalar_int();
						operation = _random_choice_from(
							minijava::ast::binary_operation_type::less_than,
							minijava::ast::binary_operation_type::less_equal,
							minijava::ast::binary_operation_type::greater_than,
							minijava::ast::binary_operation_type::greater_equal
						);
						break;
					default:
						MINIJAVA_NOT_REACHED();
					}
					return _factory.make<minijava::ast::binary_expression>()(
						operation,
						_make_expression(operand_type),
						_make_expression(operand_type)
					);
				}
				MINIJAVA_NOT_REACHED();
			}

			std::unique_ptr<minijava::ast::unary_expression>
			_make_unary_expression(const yet_another_type_type& type)
			{
				assert(!is_void(type) && is_primitive(type) && !is_array(type));
				const auto g = _enter_nested();
				if (is_integer(type)) {
					return _factory.make<minijava::ast::unary_expression>()(
						minijava::ast::unary_operation_type::minus,
						_make_integral_expression()
					);
				}
				if (is_boolean(type)) {
					return _factory.make<minijava::ast::unary_expression>()(
						minijava::ast::unary_operation_type::logical_not,
						_make_boolean_expression()
					);
				}
				MINIJAVA_NOT_REACHED();
			}

			std::unique_ptr<minijava::ast::method_invocation>
			_maybe_make_method_invocation(const yet_another_type_type& type)
			{
				const auto g = _enter_nested();
				auto candidates = std::vector<std::pair<minijava::symbol, const method_signature*>>{};
				for (const auto& kv : _class_methods) {
					for (const auto& sig : kv.second) {
						if (equal(sig.return_type, type)) {
							candidates.push_back({kv.first, &sig});
						}
					}
				}
				if (candidates.empty()) {
					return std::unique_ptr<minijava::ast::method_invocation>{};
				}
				const auto callee = _random_choice(candidates);
				auto target = std::unique_ptr<minijava::ast::expression>{};
				if (callee.first == _current_class) {
					auto thisdist = std::bernoulli_distribution{};
					if (thisdist(_engine)) {
						target = _factory.make<minijava::ast::this_ref>()();
					}
				} else {
					target = _make_target({callee.first});
				}
				auto arguments = std::vector<std::unique_ptr<minijava::ast::expression>>{};
				std::transform(
					std::begin(callee.second->parameter_types),
					std::end(callee.second->parameter_types),
					std::back_inserter(arguments),
					[self = this](auto&& yatt){ return self->_make_expression(yatt); }
				);
				return _factory.make<minijava::ast::method_invocation>()(
					std::move(target),
					callee.second->name,
					std::move(arguments)
				);
			}

			std::unique_ptr<minijava::ast::expression>
			_maybe_make_variable_access(const yet_another_type_type& type)
			{
				const auto g = _enter_nested();
				auto node = std::unique_ptr<minijava::ast::expression>{};
				auto vars = std::unordered_map<minijava::symbol, const minijava::ast::var_decl*>{};
				for (auto it = std::rbegin(_scopes); it != std::rend(_scopes); ++it) {
					for (const auto& decl : *it) {
						vars.insert({decl->name(), decl});
					}
				}
				auto candidates = std::vector<std::pair<minijava::symbol, bool>>{};
				for (const auto& var : vars) {
					if (equal(var.second->var_type(), type)) {
						candidates.push_back({var.first, false});
					}
				}
				if (!_current_class.empty()) {
					for (auto&& sig : _class_fields[_current_class]) {
						if (equal(sig.type, type)) {
							candidates.push_back({sig.name, true});
						}
					}
				}
				if (!candidates.empty()) {
					const auto cand = _random_choice(candidates);
					auto target = std::unique_ptr<minijava::ast::this_ref>{};
					if (cand.second) {
						target = _factory.make<minijava::ast::this_ref>()();
					}
					node = _factory.make<minijava::ast::variable_access>()
						(std::move(target), cand.first);
				}
				return node;
			}

			std::unique_ptr<minijava::ast::this_ref>
			_maybe_make_this_ref(const yet_another_type_type& type)
			{
				const auto g = _enter_nested();
				auto node = std::unique_ptr<minijava::ast::this_ref>{};
				if (type.rank() == 0) {
					const auto p = boost::get<minijava::symbol>(&type.name());
					if ((p != nullptr) && (*p == _current_class)) {
						node = _factory.make<minijava::ast::this_ref>()();
					}
				}
				return node;
			}

			std::unique_ptr<minijava::ast::expression>
			_make_constant(const yet_another_type_type& type)
			{
				assert(!is_void(type));
				const auto g = _enter_nested();
				if (type.rank() > 0) {
					return _factory.make<minijava::ast::null_constant>()();
				} else if (is_integer(type)) {
					return _make_integer_constant();
				} else if (is_boolean(type)) {
					return _make_boolean_constant();
				} else {
					return _factory.make<minijava::ast::null_constant>()();
				}
			}

			std::unique_ptr<minijava::ast::integer_constant>
			_make_integer_constant()
			{
				const auto g = _enter_nested();
				auto dist = std::uniform_int_distribution<std::int64_t>{
					std::numeric_limits<std::int32_t>::min(),
					std::numeric_limits<std::int32_t>::max()
				};
				const auto value = dist(_engine);
				const auto negative = (value < 0);
				const auto magnitude = negative ? -value : value;
				return _factory.make<minijava::ast::integer_constant>()
					(_pool.normalize(std::to_string(magnitude)), negative);
			}

			std::unique_ptr<minijava::ast::boolean_constant>
			_make_boolean_constant()
			{
				const auto g = _enter_nested();
				auto dist = std::bernoulli_distribution{};
				return _factory.make<minijava::ast::boolean_constant>()
					(dist(_engine));
			}

			std::unique_ptr<minijava::ast::type>
			_make_type(const bool voidok)
			{
				return _make_type(_random_type(voidok));
			}

			std::unique_ptr<minijava::ast::type>
			_make_type(const yet_another_type_type& yatt)
			{
				const auto g = _enter_nested();
				if (const auto p = boost::get<minijava::symbol>(&yatt.name())) {
					return _factory.make<minijava::ast::type>()(*p, yatt.rank());
				} else {
					return _factory.make<minijava::ast::type>()(
						boost::get<minijava::ast::primitive_type>(yatt.name()),
						yatt.rank()
					);
				}
			}

			yet_another_type_type _random_type(const bool voidok)
			{
				auto primdist = std::bernoulli_distribution{0.5};
				auto rankdist = std::geometric_distribution<std::size_t>{0.5};
				if (primdist(_engine)) {
					auto typedist = std::uniform_int_distribution<int>{voidok ? 0 : 1, 2};
					switch (typedist(_engine)) {
					case 0:
						assert(voidok);
						return {minijava::ast::primitive_type::type_void, 0};
					case 1:
						return {
							minijava::ast::primitive_type::type_boolean,
							rankdist(_engine)
						};
					case 2:
						return {
							minijava::ast::primitive_type::type_int,
							rankdist(_engine)
						};
					default:
						MINIJAVA_NOT_REACHED();
					}
				} else {
					return {_random_choice(_class_names), rankdist(_engine)};
				}
			}

			std::vector<minijava::symbol> _make_unique_names()
			{
				auto names = std::vector<minijava::symbol>{};
				std::generate_n(
					std::back_inserter(names),
					_fan_out_n(),
					[self = this](){
						const auto name = get_random_identifier(self->_engine, 0.5);
						return self->_pool.normalize(name);
					}
				);
				std::sort(
					std::begin(names),
					std::end(names),
					minijava::symbol_comparator{}
				);
				const auto end = std::unique(std::begin(names), std::end(names));
				names.erase(end, names.end());
				return names;
			}

			template <typename ContainerT>
			auto _random_choice(const ContainerT& c) -> decltype(c[c.size()])
			{
				assert(c.size() > 0);
				auto dist = std::uniform_int_distribution<std::size_t>{0, c.size() - 1};
				return c[dist(_engine)];
			}

			template <typename... Ts>
			std::common_type_t<std::decay_t<Ts>...>
			_random_choice_from(Ts&&... choices)
			{
				using common_type = std::common_type_t<std::decay_t<Ts>...>;
				using array_type = std::array<common_type, sizeof...(choices)>;
				const array_type array = {{choices...}};
				return _random_choice(array);
			}

		};  // struct ast_generator

	}  // namespace detail

	template <typename EngineT, typename PoolT>
	std::unique_ptr<minijava::ast::program>
	generate_semantic_ast(
		EngineT&               engine,
		PoolT&                 pool,
		minijava::ast_factory& factory,
		const std::size_t      limit
	)
	{
		detail::ast_generator<EngineT, PoolT> astgen{engine, pool, factory, limit};
		return astgen();
	}

}  // namespace testaux
