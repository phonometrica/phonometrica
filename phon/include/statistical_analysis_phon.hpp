#ifndef STATISTICAL_ANALYSIS_PHON_SCRIPT_INCLUDE
#define STATISTICAL_ANALYSIS_PHON_SCRIPT_INCLUDE

static const char *statistical_analysis_script = R"_(
function report_chi2(data)
	var res = chi2_test(data)
	print("Pearson's chi-squared test:")
	print("chi2 = " & res.chi2)
	print("degrees of freedom = " & res.df)
	var p = res.p
	var suffix
	if p < 0.001 then
		suffix = " (p < 0.001)"
	elsif p < 0.01 then
		suffix = " (p < 0.01)"
	elsif p < 0.05 then
		suffix = " (p < 0.05)"
	else
		suffix = " (non significant)"	
	end
	print("p-value = " & p & suffix)
end
)_";

#endif /* STATISTICAL_ANALYSIS_PHON_SCRIPT_INCLUDE */