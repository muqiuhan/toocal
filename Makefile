fmt:
	find core -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format -i

line:
	cloc tests core