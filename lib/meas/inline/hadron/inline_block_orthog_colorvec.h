// -*- C++ -*-
// $Id: inline_block_orthog_colorvec.h,v 3.1 2009-04-11 03:32:46 edwards Exp $
/*! \file
 * \brief Block orthogonalize a colorvec structure
 */

#ifndef __inline_block_orthog_colorvec_h__
#define __inline_block_orthog_colorvec_h__

#include "chromabase.h"
#include "meas/inline/abs_inline_measurement.h"
#include "io/qprop_io.h"

namespace Chroma 
{ 
  /*! \ingroup inlinehadron */
  namespace InlineBlockOrthogColorVecsEnv 
  {
    bool registerAll();

    //! Parameter structure
    /*! \ingroup inlinehadron */ 
    struct Params 
    {
      Params();
      Params(XMLReader& xml_in, const std::string& path);

      unsigned long     frequency;

      struct Param_t
      {

	struct Sources_t {
	  multi1d<int>                     spatial_mask_size;
	  multi1d<multi1d<multi1d<int> > > spatial_masks ;
	  int decay_dir;            /*!< Decay direction */

	  bool smear ;
	  GroupXML_t  smr; /*!< xml holding smearing params */
	  GroupXML_t  link_smear;  /*!< link smearing xml */
	};
	Sources_t       src  ;
	int Nhits ;
	bool OrthoNormal ;
	bool BlockOrthoNormal ;
	multi1d<int> block ;
      } param;

      struct NamedObject_t
      {
	std::string     gauge_id;      /*!< Gauge field */
	std::string     colorvec_id;   /*!< Id for color vectors */
      } named_obj;

      std::string xml_file;  // Alternate XML file pattern
    };


    //! Inline task for compute LatticeColorVector matrix elements of a propagator
    /*! \ingroup inlinehadron */
    class InlineMeas : public AbsInlineMeasurement 
    {
    public:
      ~InlineMeas() {}
      InlineMeas(const Params& p) : params(p) {}
      InlineMeas(const InlineMeas& p) : params(p.params) {}

      unsigned long getFrequency(void) const {return params.frequency;}

      //! Do the measurement
      void operator()(const unsigned long update_no,
		      XMLWriter& xml_out); 

    protected:
      //! Do the measurement
      void func(const unsigned long update_no,
		XMLWriter& xml_out); 

    private:
      Params params;
    };

  } // namespace BlockOrthogColorVecsEnv

}

#endif
