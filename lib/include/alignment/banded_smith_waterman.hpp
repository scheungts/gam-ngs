/*
 *  This file is part of GAM-NGS.
 *  Copyright (c) 2011 by Riccardo Vicedomini <rvicedomini@appliedgenomics.org>,
 *  Francesco Vezzi <vezzi@appliedgenomics.org>,
 *  Simone Scalabrin <scalabrin@appliedgenomics.org>,
 *  Lars Arverstad <lars.arvestad@scilifelab.se>,
 *  Alberto Policriti <policriti@appliedgenomics.org>,
 *  Alberto Casagrande <casagrande@appliedgenomics.org>
 *
 *  GAM-NGS is an evolution of a previous work (GAM) done by Alberto Casagrande,
 *  Cristian Del Fabbro, Simone Scalabrin, and Alberto Policriti.
 *  In particular, GAM-NGS has been adapted to work on NGS data sets and it has
 *  been written using GAM's software as starting point. Thus, it shares part of
 *  GAM's source code.
 *
 *  GAM-NGS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GAM-NGS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GAM-NGS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _BANDED_SMITH_WATERMAN_
#define _BANDED_SMITH_WATERMAN_

#include "alignment/my_alignment.hpp"
#include "assembly/contig.hpp"

#define FORCE_MAXGAP_LEN 10
#define DEFAULT_BAND_SIZE 150
#define BSW_MAX_ALIGNMENT 500000

class BandedSmithWaterman
{
    public:
        typedef long int int_type;
        typedef unsigned long int size_type;

    private:
        const ScoreType _match_score;
        const ScoreType _mismatch_score;
        const ScoreType _gap_score;
        const ScoreType _gap_ext_score;
        const size_type _band_size;

    public:

        BandedSmithWaterman();

        BandedSmithWaterman(
                const ScoreType& match_score,
                const ScoreType& mismatch_score,
                const ScoreType& gap_score,
                const ScoreType& gap_ext_score,
                const size_type& band_size
                );

        BandedSmithWaterman( const size_type& band_size );

        MyAlignment
        find_alignment(const Contig& a, size_type begin_a, size_type end_a,
                const Contig& b, size_type begin_b, size_type end_b,
				bool force_start = false, bool force_end = false ) const;
};

#endif // _BANDED_SMITH_WATERMAN_

