N_REPEATS=8
for i in {1..$N_REPEATS}
do
    python script/run_exps.py data/benchmark/test_PBS2_delay_p01 paper_exp_comparison_p01_all
    python script/run_exps.py data/benchmark/test_PBS2_delay_p03 paper_exp_comparison_p03_all
    python script/run_exps.py data/benchmark/test_PBS2_delay_p002 paper_exp_comparison_p002_all
done