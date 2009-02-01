// $Id: inline_baryon_block_matelem_w.cc,v 1.4 2009-02-01 05:34:53 kostas Exp $
/*! \file
 * \brief Inline measurement of baryon operators via colorvector matrix elements
 */

#include "handle.h"
#include "meas/inline/hadron/inline_baryon_block_matelem_w.h"
#include "meas/inline/abs_inline_measurement_factory.h"
#include "meas/smear/link_smearing_aggregate.h"
#include "meas/smear/link_smearing_factory.h"
#include "meas/glue/mesplq.h"
#include "meas/smear/disp_colvec_map.h"
#include "util/ferm/block_subset.h"
#include "util/ferm/block_couplings.h"
#include "util/ferm/subset_vectors.h"
#include "util/ferm/key_val_db.h"
#include "util/ft/sftmom.h"
#include "util/info/proginfo.h"
#include "meas/inline/make_xml_file.h"

#include "meas/inline/io/named_objmap.h"

#define COLORVEC_MATELEM_TYPE_ZERO       0
#define COLORVEC_MATELEM_TYPE_ONE        1
#define COLORVEC_MATELEM_TYPE_MONE       -1
#define COLORVEC_MATELEM_TYPE_GENERIC    10

namespace Chroma 
{ 
  /*!
   * \ingroup hadron
   *
   * @{
   */
  namespace InlineBaryonBlockMatElemEnv 
  { 
    // Reader for input parameters
    void read(XMLReader& xml, const string& path, InlineBaryonBlockMatElemEnv::Params::Param_t::Displacement_t& param)
    {
      XMLReader paramtop(xml, path);

      read(paramtop, "left", param.left);
      read(paramtop, "middle", param.middle);
      read(paramtop, "right", param.right);
    }


    // Writer for input parameters
    void write(XMLWriter& xml, const string& path, const InlineBaryonBlockMatElemEnv::Params::Param_t::Displacement_t& param)
    {
      push(xml, path);

      write(xml, "left", param.left);
      write(xml, "middle", param.middle);
      write(xml, "right", param.right);

      pop(xml);
    }


    // Reader for input parameters
    void read(XMLReader& xml, const string& path, InlineBaryonBlockMatElemEnv::Params::Param_t& param)
    {
      XMLReader paramtop(xml, path);
    
      int version;
      read(paramtop, "version", version);

      switch (version) 
      {
      case 1:
	/**************************************************************************/
	break;

      default :
	/**************************************************************************/

	QDPIO::cerr << "Input parameter version " << version << " unsupported." << endl;
	QDP_abort(1);
      }

      read(paramtop, "mom2_max", param.mom2_max);
      read(paramtop, "displacement_length", param.displacement_length);
      read(paramtop, "displacement_list", param.displacement_list);
      read(paramtop, "num_vecs", param.num_vecs);
      read(paramtop, "decay_dir", param.decay_dir);
      read(paramtop, "block", param.block);
      read(paramtop, "site_orthog_basis", param.site_orthog_basis);

      param.link_smearing  = readXMLGroup(paramtop, "LinkSmearing", "LinkSmearingType");
    }


    // Writer for input parameters
    void write(XMLWriter& xml, const string& path, const InlineBaryonBlockMatElemEnv::Params::Param_t& param)
    {
      push(xml, path);

      int version = 1;

      write(xml, "version", version);
      write(xml, "mom2_max", param.mom2_max);
      write(xml, "displacement_length", param.displacement_length);
      write(xml, "displacement_list", param.displacement_list);
      write(xml, "num_vecs", param.num_vecs);
      write(xml, "decay_dir", param.decay_dir);
      write(xml, "block", param.block);
      write(xml, "site_orthog_basis", param.site_orthog_basis);
      xml << param.link_smearing.xml;

      pop(xml);
    }

    //! Read named objects 
    void read(XMLReader& xml, const string& path, InlineBaryonBlockMatElemEnv::Params::NamedObject_t& input)
    {
      XMLReader inputtop(xml, path);

      read(inputtop, "gauge_id", input.gauge_id);
      read(inputtop, "colorvec_id", input.colorvec_id);
      read(inputtop, "baryon_op_file", input.baryon_op_file);
    }

    //! Write named objects
    void write(XMLWriter& xml, const string& path, const InlineBaryonBlockMatElemEnv::Params::NamedObject_t& input)
    {
      push(xml, path);

      write(xml, "gauge_id", input.gauge_id);
      write(xml, "colorvec_id", input.colorvec_id);
      write(xml, "baryon_op_file", input.baryon_op_file);

      pop(xml);
    }

    // Writer for input parameters
    void write(XMLWriter& xml, const string& path, const InlineBaryonBlockMatElemEnv::Params& param)
    {
      param.writeXML(xml, path);
    }
  }


  namespace InlineBaryonBlockMatElemEnv 
  { 
    // Anonymous namespace for registration
    namespace
    {
      AbsInlineMeasurement* createMeasurement(XMLReader& xml_in, 
					      const std::string& path) 
      {
	return new InlineMeas(Params(xml_in, path));
      }

      //! Local registration flag
      bool registered = false;
    }

    const std::string name = "BARYON_BLOCK_MATELEM";

    //! Register all the factories
    bool registerAll() 
    {
      bool success = true; 
      if (! registered)
      {
	success &= LinkSmearingEnv::registerAll();
	success &= TheInlineMeasurementFactory::Instance().registerObject(name, createMeasurement);
	registered = true;
      }
      return success;
    }


    //! Anonymous namespace
    /*! Diagnostic stuff */
    namespace
    {
      StandardOutputStream& operator<<(StandardOutputStream& os, const multi1d<int>& d)
      {
	if (d.size() > 0)
	{
	  os << d[0];

	  for(int i=1; i < d.size(); ++i)
	    os << " " << d[i];
	}

	return os;
      }

      StandardOutputStream& operator<<(StandardOutputStream& os, const Params::Param_t::Displacement_t& d)
      {
	os << "left= " << d.left 
	   << " middle= " << d.middle
	   << " right= " << d.right;

	return os;
      }
    }


    //----------------------------------------------------------------------------
    // Param stuff
    Params::Params()
    { 
      frequency = 0; 
      param.mom2_max = 0;
    }

    Params::Params(XMLReader& xml_in, const std::string& path) 
    {
      try 
      {
	XMLReader paramtop(xml_in, path);

	if (paramtop.count("Frequency") == 1)
	  read(paramtop, "Frequency", frequency);
	else
	  frequency = 1;

	// Read program parameters
	read(paramtop, "Param", param);

	// Read in the output propagator/source configuration info
	read(paramtop, "NamedObject", named_obj);

	// Possible alternate XML file pattern
	if (paramtop.count("xml_file") != 0) 
	{
	  read(paramtop, "xml_file", xml_file);
	}
      }
      catch(const std::string& e) 
      {
	QDPIO::cerr << __func__ << ": Caught Exception reading XML: " << e << endl;
	QDP_abort(1);
      }
    }


    void
    Params::writeXML(XMLWriter& xml_out, const std::string& path) const
    {
      push(xml_out, path);
    
      // Parameters for source construction
      write(xml_out, "Param", param);

      // Write out the output propagator/source configuration info
      write(xml_out, "NamedObject", named_obj);

      pop(xml_out);
    }


    //----------------------------------------------------------------------------
    //! Baryon operator
    struct KeyBaryonElementalOperator_t
    {
      int                t_slice;      /*!< Baryon operator time slice */
      multi1d<int>       left;         /*!< Displacement dirs of left colorvector */
      multi1d<int>       middle;       /*!< Displacement dirs of middle colorvector */
      multi1d<int>       right;        /*!< Displacement dirs of right colorvector */
      multi1d<int>       mom;          /*!< D-1 momentum of this operator */
      int b_left  ; // the block of the  left quark    
      int b_midle ; // the block for the midle quark                              
      int b_right ; // the block for the right quark   
    };

    //! Baryon operator
    struct ValBaryonElementalOperator_t
    {
      int                type_of_data; /*!< Flag indicating type of data (maybe trivial) */
      multi3d<ComplexD>  op;           /*!< Momentum projected operator */
    };


    //----------------------------------------------------------------------------
    //! Holds key and value as temporaries
    struct KeyValBaryonElementalOperator_t
    {
      SerialDBKey<KeyBaryonElementalOperator_t>  key;
      SerialDBData<ValBaryonElementalOperator_t> val;
    };


    //----------------------------------------------------------------------------
    //! BaryonElementalOperator reader
    void read(BinaryReader& bin, KeyBaryonElementalOperator_t& param)
    {
      read(bin, param.t_slice);
      read(bin, param.left);
      read(bin, param.middle);
      read(bin, param.right);
      read(bin, param.mom);
      read(bin, param.b_left);
      read(bin, param.b_midle);
      read(bin, param.b_right);
    }

    //! BaryonElementalOperator write
    void write(BinaryWriter& bin, const KeyBaryonElementalOperator_t& param)
    {
      write(bin, param.t_slice);
      write(bin, param.left);
      write(bin, param.middle);
      write(bin, param.right);
      write(bin, param.mom);
      write(bin, param.b_left);
      write(bin, param.b_midle);
      write(bin, param.b_right);
    }

    //! BaryonElementalOperator reader
    void read(XMLReader& xml, const std::string& path, KeyBaryonElementalOperator_t& param)
    {
      XMLReader paramtop(xml, path);
    
      read(paramtop, "t_slice", param.t_slice);
      read(paramtop, "left", param.left);
      read(paramtop, "middle", param.middle);
      read(paramtop, "right", param.right);
      read(paramtop, "mom", param.mom);
      read(paramtop, "left_block", param.b_left);
      read(paramtop, "middle_block", param.b_midle);
      read(paramtop, "right_block", param.b_right);
    }

    //! BaryonElementalOperator writer
    void write(XMLWriter& xml, const std::string& path, const KeyBaryonElementalOperator_t& param)
    {
      push(xml, path);

      write(xml, "t_slice", param.t_slice);
      write(xml, "left", param.left);
      write(xml, "middle", param.middle);
      write(xml, "right", param.right);
      write(xml, "mom", param.mom);
      write(xml, "left_block", param.b_left);
      write(xml, "middle_block", param.b_midle);
      write(xml, "right_block", param.b_right);
      pop(xml);
    }


    //----------------------------------------------------------------------------
    //! BaryonElementalOperator reader
    void read(BinaryReader& bin, ValBaryonElementalOperator_t& param)
    {
      read(bin, param.type_of_data);

      int n;
      read(bin, n);    // the size is always written, even if 0
      param.op.resize(n,n,n);
  
      for(int i=0; i < n; ++i)
      {
	for(int j=0; j < n; ++j)
	{
	  for(int k=0; k < n; ++k)
	  {
	    read(bin, param.op(i,j,k));
	  }
	}
      }
    }

    //! BaryonElementalOperator write
    void write(BinaryWriter& bin, const ValBaryonElementalOperator_t& param)
    {
      write(bin, param.type_of_data);

      int n = param.op.size1();  // all sizes the same
      write(bin, n);
      for(int i=0; i < n; ++i)
      {
	for(int j=0; j < n; ++j)
	{
	  for(int k=0; k < n; ++k)
	  {
	    write(bin, param.op(i,j,k));
	  }
	}
      }
    }


    //----------------------------------------------------------------------------
    //! Normalize just one displacement array
    multi1d<int> normDisp(const multi1d<int>& orig)
    {
      START_CODE();

      multi1d<int> disp;
      multi1d<int> empty; 
      multi1d<int> no_disp(1); no_disp[0] = 0;

      // NOTE: a no-displacement is recorded as a zero-length array
      // Convert a length one array with no displacement into a no-displacement array
      if (orig.size() == 1)
      {
	if (orig == no_disp)
	  disp = empty;
	else
	  disp = orig;
      }
      else
      {
	disp = orig;
      }

      END_CODE();

      return disp;
    } // void normDisp


    //----------------------------------------------------------------------------
    //! Make sure displacements are something sensible
    multi1d<Params::Param_t::Displacement_t> 
    normalizeDisplacements(const multi1d<Params::Param_t::Displacement_t>& orig_list)
    {
      START_CODE();

      multi1d<Params::Param_t::Displacement_t> displacement_list(orig_list.size());

      // Loop over displacements
      for(int n=0; n < orig_list.size(); ++n)
      {
	const Params::Param_t::Displacement_t& o = orig_list[n];
	Params::Param_t::Displacement_t&       d = displacement_list[n];

	d.left   = normDisp(o.left);
	d.middle = normDisp(o.middle);
	d.right  = normDisp(o.right);

//	QDPIO::cout << "disp[" << n << "]="
//		    << "  left= " << d.left
//		    << "  middle= " << d.middle
//		    << "  right= " << d.right
//		    << endl;
      }

      END_CODE();

      return displacement_list;
    } // void normalizeDisplacements


    //-------------------------------------------------------------------------------
    // Function call
    void 
    InlineMeas::operator()(unsigned long update_no,
			   XMLWriter& xml_out) 
    {
      // If xml file not empty, then use alternate
      if (params.xml_file != "")
      {
	string xml_file = makeXMLFileName(params.xml_file, update_no);

	push(xml_out, "BaryonBlockMatElem");
	write(xml_out, "update_no", update_no);
	write(xml_out, "xml_file", xml_file);
	pop(xml_out);

	XMLFileWriter xml(xml_file);
	func(update_no, xml);
      }
      else
      {
	func(update_no, xml_out);
      }
    }


    // Function call
    void 
    InlineMeas::func(unsigned long update_no,
		     XMLWriter& xml_out) 
    {
      START_CODE();

      StopWatch snoop;
      snoop.reset();
      snoop.start();

      
      StopWatch swiss;
			
      // Test and grab a reference to the gauge field
      XMLBufferWriter gauge_xml;
      try
      {
	TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_id);
	TheNamedObjMap::Instance().get(params.named_obj.gauge_id).getRecordXML(gauge_xml);

	TheNamedObjMap::Instance().getData< SubsetVectors<LatticeColorVector> >(params.named_obj.colorvec_id).getEvectors();
      }
      catch( std::bad_cast ) 
      {
	QDPIO::cerr << name << ": caught dynamic cast error" << endl;
	QDP_abort(1);
      }
      catch (const string& e) 
      {
	QDPIO::cerr << name << ": map call failed: " << e << endl;
	QDP_abort(1);
      }
      const multi1d<LatticeColorMatrix>& u = 
	TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_id);

      const SubsetVectors<LatticeColorVector>& eigen_source = 
	TheNamedObjMap::Instance().getData< SubsetVectors<LatticeColorVector> >(params.named_obj.colorvec_id);

      push(xml_out, "BaryonBlockMatElem");
      write(xml_out, "update_no", update_no);

      QDPIO::cout << name << ": Baryon color-vector matrix element" << endl;

      proginfo(xml_out);    // Print out basic program info

      // Write out the input
      params.writeXML(xml_out, "Input");

      // Write out the config info
      write(xml_out, "Config_info", gauge_xml);

      push(xml_out, "Output_version");
      write(xml_out, "out_version", 1);
      pop(xml_out);

      //First calculate some gauge invariant observables just for info.
      //This is really cheap.
      MesPlq(xml_out, "Observables", u);

      //
      // Initialize the slow Fourier transform phases
      //
      SftMom phases(params.param.mom2_max, false, params.param.decay_dir);

      //
      // Smear the gauge field if needed
      //
      multi1d<LatticeColorMatrix> u_smr = u;

      try
      {
	std::istringstream  xml_l(params.param.link_smearing.xml);
	XMLReader  linktop(xml_l);
	QDPIO::cout << "Link smearing type = " << params.param.link_smearing.id << endl;
	
	
	Handle< LinkSmearing >
	  linkSmearing(TheLinkSmearingFactory::Instance().createObject(params.param.link_smearing.id,
								       linktop, 
								       params.param.link_smearing.path));

	(*linkSmearing)(u_smr);
      }
      catch(const std::string& e) 
      {
	QDPIO::cerr << name << ": Caught Exception link smearing: " << e << endl;
	QDP_abort(1);
      }

      MesPlq(xml_out, "Smeared_Observables", u_smr);

      //
      // Make sure displacements are something sensible
      //
      QDPIO::cout << "Normalize displacement lengths" << endl;
      multi1d<Params::Param_t::Displacement_t> displacement_list(normalizeDisplacements(params.param.displacement_list));

      //
      // The object holding the displaced color vector maps  
      //
      DispColorVectorMap smrd_disp_vecs(params.param.displacement_length,
					u_smr,
					eigen_source.getEvectors());

      //
      // Baryon operators
      //
      // The creation and annihilation operators are the same without the
      // spin matrices.
      //
      QDPIO::cout << "Building baryon operators" << endl;

      // DB storage
      BinaryFxStoreDB< SerialDBKey<KeyBaryonElementalOperator_t>, SerialDBData<ValBaryonElementalOperator_t> > 
	qdp_db(params.named_obj.baryon_op_file, 10*1024*1024, 64*1024);

      push(xml_out, "ElementalOps");

      // Loop over all time slices for the source. This is the same 
      // as the subsets for  phases

      //Make the block Set                                                       
      Set blocks ;
      blocks.make(BlockFunc(params.param.decay_dir, params.param.block));
      
      QDPIO::cout<<"Number of Blocks: "<<blocks.numSubsets()<<endl ;
      // Loop over each operator 
      for(int l=0; l < displacement_list.size(); ++l)
      {
	StopWatch watch;

	QDPIO::cout << "Elemental operator: op = " << l << endl;

	QDPIO::cout << "displacement: " << displacement_list[l] << endl;

	// Build the operator
	swiss.reset();
	swiss.start();
	
	multi1d<DisplacedBlock> dblk(3) ;
	for(int blk_left(0);blk_left<blocks.numSubsets();blk_left++){
	  //QDPIO::cout<<"blk_left : "<<blk_left<<endl ;
	  dblk[0].blk  = blk_left ;
	  dblk[0].disp = displacement_list[l].left ;
	  for(int blk_midle(0);blk_midle< blocks.numSubsets();blk_midle++){
	    dblk[1].blk  = blk_midle ;
	    dblk[1].disp = displacement_list[l].middle ;
	    if(blocks_couple(dblk,blocks,params.param.displacement_length,2))
	      for(int blk_right(0);blk_right<blocks.numSubsets();blk_right++){
		dblk[2].blk  = blk_right ;
		dblk[2].disp = displacement_list[l].right ;
		if(blocks_couple(dblk,blocks,params.param.displacement_length)){
		  QDPIO::cout<<"Doing block: "<<blk_left<<" "<<blk_midle<<" "<<blk_right<<endl ;

		  // Big loop over the momentum projection
		  for(int mom_num = 0 ; mom_num < phases.numMom() ; ++mom_num) 
		    {
		      // The keys for the displacements for this particular elemental operator
		      // Invert the time - make it an independent key
		      multi1d<KeyValBaryonElementalOperator_t> buf(phases.numSubsets());
		      for(int t=0; t < phases.numSubsets(); ++t)
			{
			  buf[t].key.key().t_slice       = t;
			  buf[t].key.key().left          = displacement_list[l].left;
			  buf[t].key.key().middle        = displacement_list[l].middle;
			  buf[t].key.key().right         = displacement_list[l].right;
			  buf[t].key.key().mom           = phases.numToMom(mom_num);
			  buf[t].val.data().op.resize(params.param.num_vecs,params.param.num_vecs,params.param.num_vecs);
			  buf[t].key.key().b_left  = blk_left ;
			  buf[t].key.key().b_midle = blk_midle ;
			  buf[t].key.key().b_right = blk_right ;
			  
			  // Build in some optimizations. 
			  // At this very moment, optimizations turned off
			  buf[t].val.data().type_of_data = COLORVEC_MATELEM_TYPE_GENERIC;
			}
		      
		      
		      // The keys for the spin and displacements for this particular elemental operator
		      multi1d<KeyDispColorVector_t> keyDispColorVector(3);
		      
		      // Can displace each colorvector
		      keyDispColorVector[0].displacement = displacement_list[l].left;
		      keyDispColorVector[1].displacement = displacement_list[l].middle;
		      keyDispColorVector[2].displacement = displacement_list[l].right;
		      for(int i = 0 ; i <  params.param.num_vecs; ++i)
			{
			  //QDPIO::cout<<"left: "<<i<<endl ;
			  keyDispColorVector[0].colvec = i;
			  LatticeColorVector q_left = zero ;
			  q_left[blocks[blk_left]] = smrd_disp_vecs.getDispVector(keyDispColorVector[0]) ;
			  
			  for(int j = 0 ; j < params.param.num_vecs; ++j)
			    {
			      //QDPIO::cout<<"middle: "<<j<<endl ;
			      keyDispColorVector[1].colvec = j;
			      LatticeColorVector q_midle = zero ;
			      q_midle[blocks[blk_midle]] = smrd_disp_vecs.getDispVector(keyDispColorVector[1]) ;
			      for(int k = 0 ; k < params.param.num_vecs; ++k)
				{
				  //QDPIO::cout<<"right: "<<k<<endl ;
				  keyDispColorVector[2].colvec = k;
				  LatticeColorVector q_right = zero ;
				  q_right[blocks[blk_right]] = smrd_disp_vecs.getDispVector(keyDispColorVector[2]) ;
				  
				  watch.reset();
				  watch.start();
				  
				  // Contract over color indices
				  // Do the relevant quark contraction
				  // Slow fourier-transform
				  LatticeComplex lop = colorContract(q_left,q_midle,q_right);
				  // Slow fourier-transform
				  multi1d<ComplexD> op_sum = sumMulti(phases[mom_num] * lop, phases.getSet());
				  
				  watch.stop();
				  
				  for(int t=0; t < op_sum.size(); ++t)
				    {
				      buf[t].val.data().op(i,j,k) = op_sum[t];
				    }
				} // end for k
			    } // end for j
			} // end for i
		      
		      QDPIO::cout << "insert: mom_num= " << mom_num << " displacement num= " << l << endl; 
		      for(int t=0; t < phases.numSubsets(); ++t)
			{
			  qdp_db.insert(buf[t].key, buf[t].val);
			}
		    } // mom_num
		} // if blocks couple
	      }// right block loop
	  }// midle block loop
	}// left block loop
	
	swiss.stop();
	
	QDPIO::cout << "Baryon operator= " << l 
		    << "  time= "
		    << swiss.getTimeInSeconds() 
		    << " secs" << endl;

      } // for l

      pop(xml_out); // ElementalOps

      // Write the meta-data and the binary for this operator
      swiss.reset();
      swiss.start();
      {
	XMLBufferWriter file_xml;

	push(file_xml, "BaryonElementalOperators");
	write(file_xml, "lattSize", QDP::Layout::lattSize());
	write(file_xml, "decay_dir", params.param.decay_dir);
	write(file_xml, "Weights", eigen_source.getEvalues());
	write(file_xml, "Params", params.param);
	write(file_xml, "Config_info", gauge_xml);
	write(file_xml, "Op_Info",displacement_list);
	pop(file_xml);

	qdp_db.insertUserdata(file_xml.str());
      }
      swiss.stop();

      QDPIO::cout << "Baryon Operator written:"
		  << "  time= " << swiss.getTimeInSeconds() << " secs" << endl;

      // Close the namelist output file XMLDAT
      pop(xml_out);     // BaryonBlockMatElemtor

      snoop.stop();
      QDPIO::cout << name << ": total time = " 
		  << snoop.getTimeInSeconds() 
		  << " secs" << endl;

      QDPIO::cout << name << ": ran successfully" << endl;

      END_CODE();
    } // func
  } // namespace InlineBaryonBlockMatElemEnv

  /*! @} */  // end of group hadron

} // namespace Chroma
