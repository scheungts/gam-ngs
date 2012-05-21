/*
 * File:   BuildPctgFunctions.code.hpp
 * Author: vice
 *
 * Created on 4 giugno 2011, 23.29
 */

#ifndef BUILDPCTGFUNCTIONS_CODE_HPP
#define	BUILDPCTGFUNCTIONS_CODE_HPP

#include "BuildPctgFunctions.hpp"
#include "pctg/MergeInCutTailFailed.hpp"

std::list< PairedContig >& buildPctg(
        const AssemblyGraph& AG,
        const PctgBuilder& builder,
        std::list< PairedContig > &pctgList,
		const Options &options )
{
    PairedContig pctg;

    if( boost::num_vertices(AG) == 0 ) return pctgList;

    //typedef std::vector< UIntType > container;
    //container c;

    typedef boost::graph_traits<AssemblyGraph>::vertex_descriptor Vertex;
    std::list< Vertex > c;
    std::vector< UIntType > notMerged;

    try
    {
		AssemblyGraph::agTopologicalSort( AG, c ); //boost::topological_sort(AG,std::back_inserter(c));
    }
    catch( boost::not_a_dag )
    {
        AssemblyGraph newAG( AG );

        std::cerr << "With cycles" << std::endl;
        newAG.removeCycles();
        std::cerr << "Without cycles" << std::endl;
        //std::cerr << newAG.getBlocksVector() << std::endl; // FIX

        throw boost::not_a_dag();
    }

    UIntType last = *(c.rbegin());
	std::list<Block> blocks_list;

    try
    {
        //std::vector<UIntType>::reverse_iterator i;
        std::list<Vertex>::reverse_iterator i;

        i = c.rbegin();
        while( i != c.rend() )
        {
            if( !Block::shareContig(AG.getBlock(*i),AG.getBlock(last)) )
            {
                //std::cout << "not share contig: " << AG.getBlock(*i) << std::endl;
                while( notMerged.size() != 0 )
                {
                    try
                    {
						// fill "merging blocks" list
						blocks_list.clear();
						for( uint32_t i=0; i < notMerged.size(); i++ ) blocks_list.push_back( AG.getBlock(notMerged[i]) );

                        // extend pctg considering blocks in the list
                        pctg = builder.extendByBlock( pctg, blocks_list, options );
                        notMerged.clear();
                    }
                    catch( std::logic_error &e ) // path interrupted by bad alignment
                    {
                        //std::cout << "  => Logic error exception1: " << e.what() << std::endl << std::flush;

                        pctgList.push_back( pctg ); // save current pctg

						// create an empty pctg for the remaining possible merges
                        pctg = PairedContig();
                    }
                    catch( MergeInCutTailFailed &e ) // path interrupted by merging in an overwritten tail (bad alignment too)
					{
						pctgList.push_back( pctg ); // save current pctg
						notMerged.clear();

						const Block &block = blocks_list.front();
						const Frame &master_frame = block.getMasterFrame();
						const Frame &slave_frame = block.getSlaveFrame();

						std::pair<IdType,IdType> masterId( master_frame.getAssemblyId(), master_frame.getContigId() ),
											 slaveId( slave_frame.getAssemblyId(), slave_frame.getContigId() );

						pctg = PairedContig();

						if( !pctg.containsMasterCtg(masterId.first,masterId.second) ) pctg = builder.addFirstContigTo( pctg, masterId );
						else pctg = builder.addFirstSlaveContigTo( pctg, slaveId );
					}
                }
            }

            //std::cout << "adding to notMerged: " << AG.getBlock(*i).getMasterFrame().getContigId() << "-" << AG.getBlock(*i).getSlaveFrame().getContigId() << std::endl << std::flush;

            notMerged.push_back(*i);
            last = *i;

            i++;
        }

		// process remaining blocks (last merge)
        try
		{
			// fill "merging blocks" list
			blocks_list.clear();
			for( uint32_t i=0; i < notMerged.size(); i++ ) blocks_list.push_back( AG.getBlock(notMerged[i]) );

			// extend pctg considering blocks in the list
			pctg = builder.extendByBlock( pctg, blocks_list, options );
			notMerged.clear();

			pctgList.push_back( pctg );
		}
		catch( std::logic_error &e )
		{
			//std::cout << "  => Logic error exception2: " << e.what() << std::endl;
			notMerged.clear(); //notMerged.pop_front()

			pctgList.push_back( pctg );
		}
		catch( MergeInCutTailFailed &e ) // path interrupted by merging in an overwritten tail (bad alignment too)
		{
			pctgList.push_back( pctg ); // save current pctg
			notMerged.clear();

			const Block &block = blocks_list.front();
			const Frame &master_frame = block.getMasterFrame();
			const Frame &slave_frame = block.getSlaveFrame();

			std::pair<IdType,IdType> masterId( master_frame.getAssemblyId(), master_frame.getContigId() ),
			slaveId( slave_frame.getAssemblyId(), slave_frame.getContigId() );

			pctg = PairedContig();

			if( !pctg.containsMasterCtg(masterId.first,masterId.second) ) pctg = builder.addFirstContigTo( pctg, masterId );
			else pctg = builder.addFirstSlaveContigTo( pctg, slaveId );
			pctgList.push_back( pctg );
		}

    }
    catch(...)
    {
        //return PairedContig(pctgId);
    }

    return pctgList;
}


std::list< PairedContig >& buildPctg(
		ThreadedBuildPctg *tbp,
        const AssemblyGraph &ag,
        const ExtContigMemPool *masterPool,
        const ExtContigMemPool *slavePool,
        const BamTools::RefVector *masterRefVector,
        const std::vector<BamTools::RefVector> *slaveRefVector,
        std::list< PairedContig > &pctgList,
		const Options &options )
{
    PctgBuilder builder( tbp, masterPool, slavePool, masterRefVector, slaveRefVector );

    //std::cout << "Building pctg " << pctgId << std::endl << std::flush;

    pctgList.clear();
    pctgList = buildPctg( ag, builder, pctgList, options );

    return pctgList;
}


void generateSingleCtgPctgs(
        std::list<PairedContig>& pctgList,
        const std::list<IdType>& ctgIds,
        //HashContigMemPool* pctgPool,
        ExtContigMemPool* masterPool,
        BamTools::RefVector *masterRefVector,
        IdType& pctgId )
{
    std::list<IdType>::const_iterator i;
    for( i = ctgIds.begin(); i != ctgIds.end(); i++ )
    {
        PctgBuilder builder( NULL, masterPool, NULL, masterRefVector, NULL );
        PairedContig pctg = builder.initByContig( pctgId, std::make_pair((IdType)0,*i) );

        if( pctg.size() > 0 )
        {
            //pctgPool->set( pctg.name(), pctg );
            pctgList.push_back( pctg );
            pctgId++;
        }
    }
}

#endif	/* BUILDPCTGFUNCTIONS_CODE_HPP */

