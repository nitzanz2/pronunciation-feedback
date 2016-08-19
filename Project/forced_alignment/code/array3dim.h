//*****************************************************************************
/** Implements a simple 3dim array for real values
 @author Shai Shalev 
 */
template <class T>
class threeDimArray {
  
public:
  
  //-----------------------------------------------------------------------------
  /** Constructs a threeDimArray of size (M,N,K)
   @param M First dimension
   @param N Second dimension
   @param K Third dimension
   */
  //  inline threeDimArray(uint M,uint N,uint K) : _M(M), _NNNN(N), _K(K) {
  inline threeDimArray(uint M,uint N,uint K) {
    _M = M;
    _NNNN = N;
    _K = K;
    _data = new T[M*N*K];
  };
  
  //*****************************************************************************
  // reference operators
  //*****************************************************************************
  //-----------------------------------------------------------------------------
  /** Reference operator. Returns a refetence to an entry in the array
   @param m The first index to the requested entry
   @param n The second index to the requested entry
   @param k The third index to the requested entry
   @returns A reference to the requested array entry
   */
  inline T& operator() (uint m, uint n, uint k) {
    return *(_data + m + (n + k*_NNNN)*_M);
  };
  
  
  // destructor
  inline ~threeDimArray() {
    delete [] _data;
  };
  
protected:
  T* _data;  // pointer to the data
  // dimensions
  uint _M;
  uint _NNNN;
  uint _K;
};
