target=load_balance.svg newidle_balance.svg nohz_idle_balance.svg tick_load_balance.svg passive_load_balance.svg

all:$(target)

%.svg : %.gv
	dot -Tsvg $^ -o $@

clean :
	rm -rf $(target)
