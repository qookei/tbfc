#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <variant>
#include <stack>
#include <algorithm>

struct ir_op {
	enum class op_type {
		add,
		sub,

		tape_next,
		tape_prev,

		print,
		read,

		peek_tape,

		jmp_zero,
		jmp_not_zero,

		loop_begin,
		loop_end,
	};

	op_type type;
	std::variant<std::string, int> arg;
};

struct ir_generator {
	ir_generator() :loop_stack{}, loop_counter{0} {}

	void generate_ir(char c, std::vector<ir_op> &dest) {
		switch(c) {
			case '+': dest.push_back(ir_op{ir_op::op_type::add, 1}); break;

			case '-': dest.push_back(ir_op{ir_op::op_type::sub, 1}); break;

			case '>': dest.push_back(ir_op{ir_op::op_type::tape_next, 1}); break;

			case '<': dest.push_back(ir_op{ir_op::op_type::tape_prev, 1}); break;

			case '.':
				dest.push_back(ir_op{ir_op::op_type::print, 0});
				break;

			case ',': 
				dest.push_back(ir_op{ir_op::op_type::read, 0});
				break;

			case '[':
				loop_stack.push(loop_counter);
				loop_counter++;
				dest.push_back(ir_op{ir_op::op_type::peek_tape, 0});
				dest.push_back(ir_op{ir_op::op_type::jmp_zero, "end_" + std::to_string(loop_stack.top())});
				dest.push_back(ir_op{ir_op::op_type::loop_begin, loop_stack.top()});
				break;

			case ']':
				dest.push_back(ir_op{ir_op::op_type::peek_tape, 0});
				dest.push_back(ir_op{ir_op::op_type::jmp_not_zero, "begin_" + std::to_string(loop_stack.top())});
				dest.push_back(ir_op{ir_op::op_type::loop_end, loop_stack.top()});
				loop_stack.pop();
				break;
		}
	}

private:
	int loop_counter;
	std::stack<int> loop_stack;
};

std::vector<ir_op> optimizer_pass_reduce_add_sub(const std::vector<ir_op> &ir){
	size_t i = 0;
	int add_counter = 0, sub_counter = 0;
	std::vector<ir_op> new_ir;

	while(i < ir.size()) {
		if (ir[i].type != ir_op::op_type::add
			&& ir[i].type != ir_op::op_type::sub) {
			new_ir.push_back(ir[i]);
			i++;
			continue;
		}

		if (ir[i].type == ir_op::op_type::add)
			add_counter++;
		else if (ir[i].type == ir_op::op_type::sub)
			sub_counter++;

		if ((i == ir.size() - 1) || (ir[i + 1].type != ir_op::op_type::add
			&& ir[i + 1].type != ir_op::op_type::sub)) {
			int diff = add_counter - sub_counter;
			if (diff < 0)
				new_ir.push_back(ir_op{ir_op::op_type::sub, -diff});
			else if (diff > 0)
				new_ir.push_back(ir_op{ir_op::op_type::add, diff});
			add_counter = 0;
			sub_counter = 0;
		}

		i++;
	}

	return new_ir;
}

std::vector<ir_op> optimizer_pass_reduce_next_prev(const std::vector<ir_op> &ir){
	size_t i = 0;
	int next_counter = 0, prev_counter = 0;
	std::vector<ir_op> new_ir;

	while(i < ir.size()) {
		if (ir[i].type != ir_op::op_type::tape_next
			&& ir[i].type != ir_op::op_type::tape_prev) {
			new_ir.push_back(ir[i]);
			i++;
			continue;
		}

		if (ir[i].type == ir_op::op_type::tape_next)
			next_counter++;
		else if (ir[i].type == ir_op::op_type::tape_prev)
			prev_counter++;

		if ((i == ir.size() - 1) || (ir[i + 1].type != ir_op::op_type::tape_next
			&& ir[i + 1].type != ir_op::op_type::tape_prev)) {
			int diff = next_counter - prev_counter;
			if (diff < 0)
				new_ir.push_back(ir_op{ir_op::op_type::tape_prev, -diff});
			else if (diff > 0)
				new_ir.push_back(ir_op{ir_op::op_type::tape_next, diff});
			next_counter = 0;
			prev_counter = 0;
		}

		i++;
	}

	return new_ir;
}

// ebx - tape offset
// ecx - tape pointer
// esi - tape size

void emit_asm_ir(ir_op op) {
	static size_t branch_counter = 0;

	switch(op.type) {
		case ir_op::op_type::add: 
			fprintf(stderr, "\tadd byte [ebx + ecx], %d\n", std::get<int>(op.arg));
			break;
		case ir_op::op_type::sub:
			fprintf(stderr, "\tsub byte [ebx + ecx], %d\n", std::get<int>(op.arg));
			break;
		case ir_op::op_type::loop_begin:
			fprintf(stderr, "begin_%d:\n", std::get<int>(op.arg));
			break;
		case ir_op::op_type::loop_end:
			fprintf(stderr, "end_%d:\n", std::get<int>(op.arg));
			break;
		case ir_op::op_type::tape_next:
			fprintf(stderr, "\tadd ebx, %d\n", std::get<int>(op.arg));
			fprintf(stderr, "\txor edx, edx\n");
			fprintf(stderr, "\tmov eax, ebx\n");
			fprintf(stderr, "\tdiv esi\n");
			fprintf(stderr, "\tmov ebx, edx\n");
			break;
		case ir_op::op_type::tape_prev:
			fprintf(stderr, "\tsub ebx, %d\n", std::get<int>(op.arg));
			fprintf(stderr, "\ttest ebx, ebx\n");
			fprintf(stderr, "\tjns .L%lu\n", branch_counter);
			fprintf(stderr, "\tadd ebx, 30000\n");
			fprintf(stderr, ".L%lu:\n", branch_counter);
			branch_counter++;
			break;
		case ir_op::op_type::peek_tape:
			fprintf(stderr, "\tmov al, [ebx + ecx]\n");
			fprintf(stderr, "\ttest al, al\n");
			break;
		case ir_op::op_type::read:
			fprintf(stderr, "\tcall impl_read\n");
			break;
		case ir_op::op_type::print:
			fprintf(stderr, "\tcall impl_print\n");
			break;
		case ir_op::op_type::jmp_not_zero:
			fprintf(stderr, "\tjnz %s\n", std::get<std::string>(op.arg).c_str());
			break;
		case ir_op::op_type::jmp_zero:
			fprintf(stderr, "\tjz %s\n", std::get<std::string>(op.arg).c_str());
			break;
	}
}

void emit_asm_prologue() {
	fprintf(stderr, "bits 32\n");
	fprintf(stderr, "global _start\n");
	fprintf(stderr, "section .data\n");
	fprintf(stderr, "_tape:\n");
	fprintf(stderr, "\ttimes 30000 db 0\n\n");
	fprintf(stderr, "section .text\n");
	fprintf(stderr, "_start:\n");
	fprintf(stderr, "\txor ebx, ebx\n");
	fprintf(stderr, "\tmov ecx, _tape\n");
	fprintf(stderr, "\tmov esi, 30000\n");
}

void emit_asm_epilogue() {
	fprintf(stderr, "\tmov eax, 1\n");
	fprintf(stderr, "\txor ebx, ebx\n");
	fprintf(stderr, "\tint 0x80\n\n");
	fprintf(stderr, "impl_print:\n");
	fprintf(stderr, "\tpushad\n");
	fprintf(stderr, "\tmov eax, 4\n");
	fprintf(stderr, "\tmov edx, 1\n");
	fprintf(stderr, "\tadd ecx, ebx\n");
	fprintf(stderr, "\tmov ebx, 1\n");
	fprintf(stderr, "\tint 0x80\n");
	fprintf(stderr, "\tpopad\n");
	fprintf(stderr, "\tret\n\n");

	fprintf(stderr, "impl_read:\n");
	fprintf(stderr, "\tpushad\n");
	fprintf(stderr, "\tmov eax, 3\n");
	fprintf(stderr, "\tmov edx, 1\n");
	fprintf(stderr, "\tadd ecx, ebx\n");
	fprintf(stderr, "\tmov ebx, 1\n");
	fprintf(stderr, "\tint 0x80\n");
	fprintf(stderr, "\tpopad\n");
	fprintf(stderr, "\tret\n");
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: <file>\n");
		return 1;
	}

	ir_generator gen;

	std::vector<ir_op> ir;

	std::ifstream file(argv[1]);
	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string source = buffer.str();

	for (char c : source) {
		gen.generate_ir(c, ir);
	}

	ir = optimizer_pass_reduce_add_sub(ir);
	ir = optimizer_pass_reduce_next_prev(ir);

	emit_asm_prologue();

	for (auto &op : ir)
		emit_asm_ir(op);

	emit_asm_epilogue();

	return 0;
}
