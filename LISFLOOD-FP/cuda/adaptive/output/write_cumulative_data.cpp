#include "write_cumulative_data.h"

void write_cumulative_data
(
	const clock_t           start,
	const real&             current_time,
	const real&             time_mra,
	const real&             time_solver,
	const real&             dt,
	const int&              num_cells,
	const SimulationParams& sim_params,
	const PlottingParams&   plot_params,
	const bool              first_t_step
)
{
	char fullpath[255] = {'\0'};

	sprintf(fullpath, "%s%c%s%s", plot_params.dirroot, '/', plot_params.resroot, ".cumu");
	
	FILE* cumulative_input = fopen(fullpath, (first_t_step) ? "w" : "a");

	if (NULL == cumulative_input)
	{
		fprintf(stderr, "Error in opening cumulative simulation data file.");
		exit(-1);
	}

	if (first_t_step) fprintf(cumulative_input, "simtime,runtime_mra,runtime_solver,runtime_total,dt,reduction\n");

	const clock_t end = clock();

	const real compression = ( C(100.0) - C(100.0) * num_cells / (sim_params.xsz * sim_params.ysz) ) / C(100.0);

	fprintf
	(
		cumulative_input,
		 "%" NUM_FRMT
		",%" NUM_FRMT
		",%" NUM_FRMT
		",%" NUM_FRMT
		",%" NUM_FRMT
		",%" NUM_FRMT
		"\n",
		current_time,
		time_mra,
		time_solver,
		time_mra + time_solver,
		dt,
		compression
	);

	fclose(cumulative_input);
}