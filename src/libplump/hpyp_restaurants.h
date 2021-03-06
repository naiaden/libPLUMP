/*
 * Copyright 2009, 2010 Jan Gasthaus (j.gasthaus@gatsby.ucl.ac.uk)
 * 
 * This file is part of libPLUMP.
 * 
 * libPLUMP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libPLUMP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with libPLUMP.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HPYP_RESTAURANTS_H
#define HPYP_RESTAURANTS_H


#include "libplump/config.h"
#include "libplump/pool.h"
#include "libplump/node_manager.h" // for IPayloadFactory
#include "libplump/serialization.h"
#include "libplump/hpyp_restaurant_interface.h"

namespace gatsby { namespace libplump {

////////////////////////////////////////////////////////////////////////////////
////////////////////////   CLASS DECLARATIONS   ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * An HPYP restaurant that stores the full seating arrangement using a
 * straightforward STL data structure (see nested struct Payload).
 *
 * For each type, the restaurant stores the number of customers sitting around
 * each individual table. The total number of customers per type as well as the
 * overall total number of customers and tables are also stored for fast
 * lookup.
 *
 * This restaurant allows for fast insertion and removal of customers, but is
 * not very memory efficient.
 */
class SimpleFullRestaurant : public IAddRemoveRestaurant {
  public:

    SimpleFullRestaurant() : payloadFactory() {}


    ~SimpleFullRestaurant() {}

    l_type getC(void* payloadPtr, e_type type) const;
    l_type getC(void* payloadPtr) const;
    l_type getT(void* payloadPtr, e_type type) const;
    l_type getT(void* payloadPtr) const;
    
    double computeProbability(void*  payloadPtr,
                              e_type type, 
                              double parentProbability,
                              double discount, 
                              double concentration) const;
    
    TypeVector getTypeVector(void* payloadPtr) const;
    
    const IPayloadFactory& getFactory() const;
    
    void updateAfterSplit(void* longerPayloadPtr, 
                          void* shorterPayloadPtr, 
                          double discountBeforeSplit, 
                          double discountAfterSplit, 
                          bool parentOnly = false) const;

    bool addCustomer(void*  payloadPtr, 
                     e_type type, 
                     double parentProbability, 
                     double discount, 
                     double concentration,
                     void* additionalData = NULL) const;

    bool removeCustomer(void* payloadPtr, 
                        e_type type,
                        double discount,
                        void* additionalData) const;

    void* createAdditionalData(void* payloadPtr, 
                               double discount, 
                               double concentration) const;

    void freeAdditionalData(void* additionalData) const;
    
    std::string toString(void* payloadPtr) const;
    
    bool checkConsistency(void* payloadPtr) const;
    
    /**
     * Construct a SimpleFullRestaurant::Payload from any other type of
     * payload by resampling a full seating arrangement for each type.
     */
    void* newPayloadFromOther(const IHPYPBaseRestaurant& fromRestaurant, 
                                     void* otherPayload, 
                                     double discount) const;

  private:
    
    class Payload : public PoolObject<Payload> {
      public:
        typedef std::pair<l_type, std::vector<l_type> > Arrangement;
        typedef std::map<e_type, Arrangement> TableMap;

        Payload() : tableMap(), sumCustomers(0), sumTables(0) {}

        TableMap tableMap;
        l_type sumCustomers;
        l_type sumTables;

        void serialize(InArchive & ar, const unsigned int version);
        void serialize(OutArchive & ar, const unsigned int version);
    };
    
    class PayloadFactory : public IPayloadFactory {
      
      void* make() const {
        return new Payload();
      };
      
      void recycle(void* payloadPtr) const {
        delete (Payload*)payloadPtr;
      }
    
      void save(void* payloadPtr, OutArchive& oa) const;
      void* load(InArchive& ia) const;
    };

    const PayloadFactory payloadFactory;
};


/**
 * An HPYP restaurant that stores the seating arrangement in histogram form
 * as suggested in 
 * "A note on the implementation of Hierarchical Dirichlet Processes". 
 * P. Blunsom et al., Proceedings of ACL, 2009
 */
class HistogramRestaurant : public IAddRemoveRestaurant {
  public:

    HistogramRestaurant() : payloadFactory() {}

    ~HistogramRestaurant() {}

    l_type getC(void* payloadPtr, e_type type) const;
    l_type getC(void* payloadPtr) const;
    l_type getT(void* payloadPtr, e_type type) const;
    l_type getT(void* payloadPtr) const;
    
    double computeProbability(void*  payloadPtr,
                              e_type type, 
                              double parentProbability,
                              double discount, 
                              double concentration) const;
    
    TypeVector getTypeVector(void* payloadPtr) const;
    
    const IPayloadFactory& getFactory() const;
    
    void updateAfterSplit(void* longerPayloadPtr,
                          void* shorterPayloadPtr,
                          double discountBeforeSplit, 
                          double discountAfterSplit, 
                          bool parentOnly = false) const;

    bool addCustomer(void*  payloadPtr, 
                     e_type type, 
                     double parentProbability, 
                     double discount, 
                     double concentration,
                     void* additionalData = NULL) const;

    bool removeCustomer(void* payloadPtr, 
                        e_type type,
                        double discount,
                        void* additionalData) const;

    void* createAdditionalData(void* payloadPtr, 
                               double discount, 
                               double concentration) const;

    void freeAdditionalData(void* additionalData) const;
    
    std::string toString(void* payloadPtr) const;
    
    bool checkConsistency(void* payloadPtr) const;
    
    /**
     * Construct a SimpleFullRestaurant::Payload from any other type of
     * payload by resampling a full seating arrangement for each type.
     */
    void* newPayloadFromOther(const IHPYPBaseRestaurant& fromRestaurant, 
                                     void* otherPayload, 
                                     double discount) const;

  private:
    
    class Payload : public PoolObject<Payload> {
      public:
        typedef std::map<l_type, l_type> Histogram;

        struct Arrangement {
          Arrangement() : cw(0), tw(0), histogram() {}
          l_type cw;
          l_type tw;
          Histogram histogram;
          
          void serialize(InArchive & ar, const unsigned int version);
          void serialize(OutArchive & ar, const unsigned int version);
        };
        
        typedef std::map<e_type, Arrangement> TableMap;

        Payload() : tableMap(), sumCustomers(0), sumTables(0) {}

        TableMap tableMap;
        l_type sumCustomers;
        l_type sumTables;

        void serialize(InArchive & ar, const unsigned int version);
        void serialize(OutArchive & ar, const unsigned int version);
    };
    
    class PayloadFactory : public IPayloadFactory {
      
      void* make() const {
        return new Payload();
      };
      
      void recycle(void* payloadPtr) const {
        delete (Payload*)payloadPtr;
      }
      
      void save(void* payloadPtr, OutArchive& oa) const;
      void* load(InArchive& ia) const;
    };

    const PayloadFactory payloadFactory;
};



/**
 * Base class for restaurants based on the "compact representation" that stores
 * only the total number of customers and the total number of tables per type, 
 * but the not the number of customers at each table.
 *
 * This representation is "minimal" in the sense that it only stores the 
 * information necessary for computing the predictive distribution.
 *
 * However, removing customers in this representation is non-trivial (and
 * computationally expensive), and must be implemented in some way in derived
 * classes.
 */
class BaseCompactRestaurant : public IAddRemoveRestaurant {
  public:
    BaseCompactRestaurant() : payloadFactory() {}

    virtual ~BaseCompactRestaurant() {}

    l_type getC(void* payloadPtr, e_type type) const;
    l_type getC(void* payloadPtr) const;
    l_type getT(void* payloadPtr, e_type type) const;
    l_type getT(void* payloadPtr) const;
    
    double computeProbability(void*  payloadPtr,
                              e_type type, 
                              double parentProbability,
                              double discount, 
                              double concentration) const;
    
    TypeVector getTypeVector(void* payloadPtr) const;
    
    const IPayloadFactory& getFactory() const;
    
    void updateAfterSplit(void* longerPayloadPtr, 
                          void* shorterPayloadPtr, 
                          double discountBeforeSplit, 
                          double discountAfterSplit, 
                          bool parentOnly = false) const;

    bool addCustomer(void*  payloadPtr, 
                     e_type type, 
                     double parentProbability, 
                     double discount, 
                     double concentration,
                     void*  additionalData = NULL) const;
    
    std::string toString(void* payloadPtr) const;
    
    bool checkConsistency(void* payloadPtr) const;

  protected:
    
    class Payload : public PoolObject<Payload> {
      public:
        // per type: cw and tw
        typedef std::pair<int, int> Arrangement;
        typedef MiniMap<e_type, Arrangement> TableMap;

        Payload() : tableMap(), sumCustomers(0), sumTables(0) {}

        TableMap tableMap;
        l_type sumCustomers;
        l_type sumTables;
      
        void serialize(InArchive & ar, const unsigned int version);
        void serialize(OutArchive & ar, const unsigned int version);
    };
    
    class PayloadFactory : public IPayloadFactory {
      public:
        void* make() const {
          return new Payload();
        };
        
        void recycle(void* payloadPtr) const {
          delete (Payload*)payloadPtr;
        }
      
        void save(void* payloadPtr, OutArchive& oa) const;
        void* load(InArchive& ia) const;
    };

    const PayloadFactory payloadFactory;
}; // BaseCompactRestaurant



class ReinstantiatingCompactRestaurant : public BaseCompactRestaurant {
  public:
    ReinstantiatingCompactRestaurant() 
        : BaseCompactRestaurant(), fullRestaurant() {}
    
    bool addCustomer(void*  payloadPtr, 
                     e_type type, 
                     double parentProbability, 
                     double discount, 
                     double concentration,
                     void*  additionalData) const;

    bool removeCustomer(void* payloadPtr, 
                        e_type type,
                        double discount,
                        void* additionalData) const;

    void* createAdditionalData(void* payloadPtr, 
                               double discount, 
                               double concentration) const;

    void freeAdditionalData(void* additionalData) const;

  private:
    const SimpleFullRestaurant fullRestaurant;
};



class StirlingCompactRestaurant : public BaseCompactRestaurant {
  public:
    StirlingCompactRestaurant() 
        : BaseCompactRestaurant() {}

    bool removeCustomer(void* payloadPtr, 
                        e_type type,
                        double discount,
                        void* additionalData) const;

    void* createAdditionalData(void* payloadPtr, 
                               double discount, 
                               double concentration) const;

    void freeAdditionalData(void* additionalData) const;
};



class ExpectedTablesCompactRestaurant : public StirlingCompactRestaurant {
  public:
    ExpectedTablesCompactRestaurant() 
        : StirlingCompactRestaurant() {}
   
    double computeProbability(void*  payloadPtr,
                              e_type type, 
                              double parentProbability,
                              double discount, 
                              double concentration) const;
    
};



class KneserNeyRestaurant : public IAddRemoveRestaurant {
  public:

    KneserNeyRestaurant() : payloadFactory() {}


    ~KneserNeyRestaurant() {}

    l_type getC(void* payloadPtr, e_type type) const;
    l_type getC(void* payloadPtr) const;
    l_type getT(void* payloadPtr, e_type type) const;
    l_type getT(void* payloadPtr) const;
    
    double computeProbability(void*  payloadPtr,
                              e_type type, 
                              double parentProbability,
                              double discount, 
                              double concentration) const;
    
    TypeVector getTypeVector(void* payloadPtr) const;
    
    const IPayloadFactory& getFactory() const;
    
    void updateAfterSplit(void* longerPayloadPtr, 
                          void* shorterPayloadPtr, 
                          double discountBeforeSplit, 
                          double discountAfterSplit, 
                          bool parentOnly = false) const;

    bool addCustomer(void*  payloadPtr, 
                     e_type type, 
                     double parentProbability, 
                     double discount, 
                     double concentration,
                     void* additionalData = NULL) const;

    bool removeCustomer(void* payloadPtr, 
                        e_type type,
                        double discount,
                        void* additionalData) const;

    void* createAdditionalData(void* payloadPtr, 
                               double discount, 
                               double concentration) const;

    void freeAdditionalData(void* additionalData) const;
    
    std::string toString(void* payloadPtr) const;
    
    bool checkConsistency(void* payloadPtr) const;
    
  private:
    
    class Payload : public PoolObject<Payload> {
      public:
        // per type: cw
        typedef std::map<e_type, l_type> TableMap;

        Payload() : tableMap(), sumCustomers(0) {}

        TableMap tableMap;
        l_type sumCustomers;
      
        void serialize(InArchive & ar, const unsigned int version);
        void serialize(OutArchive & ar, const unsigned int version);
    };
    
    class PayloadFactory : public IPayloadFactory {
      
      void* make() const {
        return new Payload();
      };
      
      void recycle(void* payloadPtr) const {
        delete (Payload*)payloadPtr;
      }
    
      void save(void* payloadPtr, OutArchive& oa) const;
      void* load(InArchive& ia) const;
    };

    const PayloadFactory payloadFactory;
};


////////////////////////////////////////////////////////////////////////////////
///////////////////   INLINE FUNCTIONS   ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

inline double computeHPYPPredictive(
    int cw, int tw, int c, int t, 
    double parentProbability, double discount, double concentration) {
  if (c==0) {
    return parentProbability;
  } else { 
    return (cw - discount*tw  
            + (concentration + discount*t)*parentProbability
           ) / (c + concentration);
  }
}


}} // namespace gatsby::libplump
#endif
