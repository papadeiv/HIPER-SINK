//---------------------------------------------------------------------------------------
// File:      src/DatIo.cpp
//
// Library:   QUASIMONT-QUAdrature of SIngular polynomials using MONomial Transformations:
//                      a C++ library for high precision integration of singular 
//                      polynomials of non-integer degree
//
// Authors:   Guido Lombardi, Davide Papapicco
//
// Institute: Politecnico di Torino
//            C.so Duca degli Abruzzi, 24 - Torino (TO), Italia
//            Department of Electronics and Telecommunications (DET)
//            Electromagnetic modelling and applications Research Group
//---------------------------------------------------------------------------------------

#include "Quasimont.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
//       FUNCTION: [n, {lambda_min, lambda_max}] = getInputData(muntz_sequence, 
//                                                              coeff_sequence)
//                
//        INPUT: - muntz_sequence = sequence of real exponents of the polynomial
//               - coeff_sequence = sequence of real coefficients of the polynomial
//
//       OUTPUT: - n = output of function 'computeNumNodes'
//               - lambda_min = minimum exponent in the input "muntz_sequence"
//               - lambda_max = maximum exponent in the input "muntz_sequence"
//
//    DESCRIPTION: the user-input polynomial is provided to the library via Main.cpp; the
//                 polynomial itself is specified via a unordered sequence of 
//                 coefficients and exponents of the various monomials in the polynomial.
//                 Once those input are read, checks have to be made in order to validate
//                 the proper functioning of the library; those are:
//                    - the number of exponents and the number of coefficients coincide;
//                    - the input polynomial is at least a binomial (otherwise the 
//                      routine further CLI user-input is required, see lines 86~93 in
//                      the 'src/MonMap.cpp' file); 
//                    - lambda_min > -1 (otherwise the program exits);
//                 Once those checks are ran the exponents' sequence is sorted locally
//                 and lambda_min/lambda_max are thus identified and outputted alongside
//                 the associated number of nodes computed by the function 
//                 'computeNumNodes' (see lines 110~175 in the 'src/MonMap.cpp' file).
//
/////////////////////////////////////////////////////////////////////////////////////////

template<typename type>
std::tuple<int, std::vector<float128>> manageData(std::vector<type>& muntz_sequence, std::vector<type>& coeff_sequence)
{
  // Print initial message and input polynomial
  std::cout << std::endl;
  std::cout << "    |――――――――――――――――――――――――――――――――――|\n"
            << "    |          ** QUASIMONT **         |\n"
            << "    |  ** MONOMIAL QUADRATURE RULE **  |\n"
            << "    |――――――――――――――――――――――――――――――――――|\n";

  if(muntz_sequence.size()==coeff_sequence.size())
  {
    std::cout << "\n\n Input polynomial p(x) = ";
    for(int k=0; k < muntz_sequence.size(); k++)
    {
      if(coeff_sequence[k]>0)
      {
        std::cout << " +";
      }
      else
      {
        std::cout << " ";
      }

      if(coeff_sequence[k]==1)
      {
        std::cout << std::setprecision(std::numeric_limits<float>::max_digits10)
                  << "x^("
                  << muntz_sequence[k] << ") ";
      }
      else
      {
        std::cout << std::setprecision(std::numeric_limits<float>::max_digits10)
                  << coeff_sequence[k] << "*x^(" << muntz_sequence[k] << ")";
      }
    }
    std::cout << std::endl;
  }
  else
  {
    std::cout << "\n   ** ERROR ** The number of exponents doesn't match the number of coefficients in the input polynomial\n";
    exit(1);
  }

  // Check compliance of the input Muntz sequence
  int num_nodes, n_min;
  float128 lambda_max, lambda_min;
  bool compute_n_min = true;

  if(muntz_sequence.size()==1)
  {
    if(muntz_sequence[0] <= -1.0)
    {
      std::cout << "\n   ** ERROR ** Lambda_min has to be (strictly) greater than -1 to be in a Muntz sequence\n";
      exit(1);
    }

    float128 additional_lambda;

    std::string input;
    std::cout << "\n ** WARNING ** Your input is a monomial of non-integer degree."
              << "\n               QUASIMONT needs a binomial for double-precision quadrature."
              << "\n               How do you proceed? ['nodes' for n_min ~ 'lambda' for lambda_max]"
              << "\n               Input: ";
    std::cin >> input;

    if(input.compare("nodes") == 0)
    {
      std::cout << "\n\nPlease specify the desired number of quadrature nodes (number must be even): ";
      std::cin >> num_nodes;

      if(num_nodes % 2 == 0)
      {
        compute_n_min = false;

        float128 local_lambda_min = static_cast<float128>(muntz_sequence[0]);
        additional_lambda = computeLambdaMax(local_lambda_min, num_nodes);
      }
      else
      {
        std::cout << "\n   ** ERROR ** The number of nodes (n_min) must be even\n";
        exit(1);
      }
    }
    else if(input.compare("lambda") == 0)
    {
      std::cout << "\n\nPlease enter the exponent value with '.' separating the decimal digits from the integer part [the more decimal digits the better the precision is]: ";
      std::cin >> additional_lambda;
    }
    else
    {
      std::cout << "\n   ** ERROR ** Your Input must be either 'nodes' or 'lambda'"
                << "\n               Please refer to Section 2.4 of doc/UserManual.pdf\n";
      exit(1);
    }

    muntz_sequence.push_back(static_cast<type>(additional_lambda));
    coeff_sequence.push_back(1.0);
  }

  // Sort the input Muntz sequence to extract lambda_min and lambda_max
  std::vector<type> loc_muntz_seq = muntz_sequence;
  sort(loc_muntz_seq.begin(), loc_muntz_seq.end());
  if(loc_muntz_seq[0] <= -1.0)
  {
    std::cout << "\n   ** ERROR ** Lambda_min has to be (strictly) greater than -1 to be in a Muntz sequence\n";
    exit(1);
  }
  
  if(loc_muntz_seq.size()==1)
  {
    if(loc_muntz_seq[0]>0)
    {
      lambda_min = static_cast<float128>(0.0);
      lambda_max = lambda_min = static_cast<float128>(loc_muntz_seq[0]);
    }
    else
    {
      lambda_min = static_cast<float128>(loc_muntz_seq[0]);
      lambda_max = static_cast<float128>(0.0);
    }
  }
  else
  {
    lambda_min = static_cast<float128>(loc_muntz_seq[0]);
    lambda_max = static_cast<float128>(loc_muntz_seq.back());
  }
  
  // Print on-screen the inputs recap information
  std::cout << "\n ** Accepted sequence of exponents ** \n";
  std::cout << std::setprecision(std::numeric_limits<float>::max_digits10)
            << "    {" << muntz_sequence[0];
  for(int k=1; k < muntz_sequence.size(); k++)
  {
    std::cout << std::setprecision(std::numeric_limits<float>::max_digits10)
              << ", " << muntz_sequence[k];
  }
  std::cout << "}";
  std::cout << std::setprecision(std::numeric_limits<float>::max_digits10)
            << "\n ** Lambda_min = " << lambda_min 
            << ", Lambda_max = " << lambda_max << " **"
            << std::endl;
  
  // Compute, or stream through, the minimum number of quadrature nodes n_min
  if(compute_n_min)
  {
    n_min = computeNumNodes(lambda_min, lambda_max);
  }
  else
  {
    n_min = num_nodes;
  }

  // Return the outputs
  std::vector<float128> lambdas = {lambda_min, lambda_max};
  return std::make_tuple(n_min, lambdas);
}
template std::tuple<int, std::vector<float128>> manageData<float128>(std::vector<float128>& muntz_sequence, std::vector<float128>& coeff_sequence);
template std::tuple<int, std::vector<float128>> manageData<double>(std::vector<double>& muntz_sequence, std::vector<double>& coeff_sequence);

/////////////////////////////////////////////////////////////////////////////////////////
//
//       FUNCTION: [n_min, {beta_min, beta_max}] = retrieveMonData(n)
//                
//        INPUT: - n = output of function 'getInputData'
//
//       OUTPUT: - n_min = minimum possible (even) number of nodes from the 
//                         'data/TabulatedErrorValues.csv' file
//               - beta_min = minimum value for the exponent of the post-map polynomial
//               - beta_max = maximum value for the exponent of the post-map polynomial
//
//    DESCRIPTION: the monomial transformation gamma: [0,1] -> [0,1] is uniquely 
//                 identified by its order r which in turn requires the knowledge of
//                 beta_min/beta_max, alongside lambda_min/lambda_max, to be computed
//                 (see lines 178~211 in the 'src/MonMap.cpp' file). This method scans
//                 the tabulated vales in the 'data/TabulatedErrorValues.csv' file to
//                 extract the beta_min/beta_max and n_min required by the monomial
//                 quadrature rule according to the specified number of nodes as either
//                 computed by the function 'computeNumNodes' (see line 243) or provided
//                 as user-input (see lines 168~199, 247).
//
/////////////////////////////////////////////////////////////////////////////////////////

std::tuple<int, std::vector<float128>> streamMonMapData(const int& comp_num_nodes)
{
  int n_min;
  float128 beta_min, beta_max;

  std::ifstream datafile;
  datafile.open("../data/TabulatedErrorValues.csv");

  std::string line, column;
  while(std::getline(datafile, line))
  {
    std::stringstream column_string(line);
    std::vector<std::string> row;
    while(std::getline(column_string, column, ','))
    {
      row.push_back(column);
    }

    n_min = stoi(row[0]);

    if(n_min >= comp_num_nodes)
    {
      beta_min = static_cast<float128>(row[1].substr(0, row[1].size() - 1));
      beta_max = static_cast<float128>(row[2].substr(0, row[2].size() - 1));

      std::cout << std::setprecision(std::numeric_limits<float>::max_digits10)
                << " ――――――――――――――――――――――――――――――――――――――――――――――――――"
                << "\n ** N_min = " << comp_num_nodes
                << "\n ** Beta_min = " << beta_min
                << ", Beta_max = " << beta_max
                << " **";

      break;
    }
  }
  std::vector<float128> betas = {beta_min, beta_max};
  return std::make_tuple(n_min, betas);
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//       FUNCTION: degradeData()
//                
//          INPUT: - [{new_x, new_w, old_x, old_w}] = output of function
//                                                       'computeParams'
//                 - muntz_sequence = sequence of real exponents of the polynomial
//                 - coeff_sequence = sequence of real coefficients of the polynomial
//
//         OUTPUT: no outputs
//
//    DESCRIPTION: TBA
//
/////////////////////////////////////////////////////////////////////////////////////////

template<typename type>
void optimiseData(std::tuple<std::vector<float128>, std::vector<float128>, std::vector<float128>, std::vector<float128>>& quad_params, std::vector<type>& muntz_sequence, std::vector<type>& coeff_sequence)
{
  const double ACC = 1*EPS;
  bool print_primitive = false;
  
  float128 I34_new = computeQuadGl(std::get<0>(quad_params), std::get<1>(quad_params), muntz_sequence, coeff_sequence);
  float128 E34_new = computeExactError(I34_new, muntz_sequence, coeff_sequence, print_primitive);

  /*std::cout << "\n\nI_n(f) with float128 = "
            << std::setprecision(std::numeric_limits<float128>::max_digits10)
            << In_34
            << "\nE_n(f) with float128 = "
            << En_34 << std::endl;*/

  if(E34_new <= ACC)
  {
    // Degrade G-L parameters to double
    std::vector<double> double_nodes = castVector(std::get<0>(quad_params), std::numeric_limits<double>::epsilon());
    std::vector<double> double_weights = castVector(std::get<1>(quad_params), std::numeric_limits<double>::epsilon());
    // Compute I_n(f) and E_n(f) with double precise G-L parameters
    float128 I16_new = computeQuadGl(double_nodes, double_weights, muntz_sequence, coeff_sequence);
    float128 E16_new = computeExactError(I16_new, muntz_sequence, coeff_sequence, print_primitive);

    /*std::cout << "\nI_n(f) with double = "
              << std::setprecision(std::numeric_limits<float128>::max_digits10)
              << In_16
              << "\nE_n(f) with double = "
              << En_16 << std::endl;*/

    if(E16_new <= ACC)
    {// Nodes and weights succefully optimised float128 -> double
      std::cout << " ――――――――――――――――――――――――――――――――――――――――――――――――――"
                << "\n ** Using double f.p. format for nodes and weights **"
                << std::endl;
      print_primitive = true;
      // Degrade classical G-L parameters with double format
      std::vector<double> old_nodes = castVector(std::get<2>(quad_params), std::numeric_limits<double>::epsilon());
      std::vector<double> old_weights = castVector(std::get<3>(quad_params), std::numeric_limits<double>::epsilon());
      // Compute classical G-L quadrature with double format parameters
      float128 I16_old = computeQuadGl(old_nodes, old_weights, muntz_sequence, coeff_sequence);
      float128 E16_old = computeExactError(I16_old, muntz_sequence, coeff_sequence, print_primitive);
      // Export all the results {In, En, I, E} in double
      exportNewData(double_nodes, double_weights, {I16_new, E16_new, I16_old, E16_old});
      // Print closing message and return
      std::cout << "\n\n ** QUASIMONT HAS TERMINATED **\n";
      return;
    }
  }
  std::cout << " ――――――――――――――――――――――――――――――――――――――――――――――――――"
            << "\n ** Using quadruple f.p. format for nodes and weights **"
            << std::endl;
  print_primitive = true;
  // Compute classical G-L quadrature with float128 format parameters
  float128 I34_old = computeQuadGl(std::get<2>(quad_params), std::get<3>(quad_params), muntz_sequence, coeff_sequence);
  float128 E34_old = computeExactError(I34_old, muntz_sequence, coeff_sequence, print_primitive);
  // Export all the results {In, En, I, E} in float128
  exportNewData(std::get<0>(quad_params), std::get<1>(quad_params), {I34_new, E34_new, I34_old, E34_old});
  // Print closing message
  std::cout << "\n\n ** QUASIMONT HAS TERMINATED **\n";
}
template void optimiseData(std::tuple<std::vector<float128>, std::vector<float128>, std::vector<float128>, std::vector<float128>>& quad_params, std::vector<float128>& muntz_sequence, std::vector<float128>& coeff_sequence); 
template void optimiseData(std::tuple<std::vector<float128>, std::vector<float128>, std::vector<float128>, std::vector<float128>>& quad_params, std::vector<double>& muntz_sequence, std::vector<double>& coeff_sequence); 


/////////////////////////////////////////////////////////////////////////////////////////
//
//       FUNCTION: exportNewData({lambda_min, lambda_max}, [n_min, {beta_min, beta_max}],
//                            [], {post_map_integral, pre_map_integral}, r)
//                
//          INPUT: - {lambda_min, lambda_max} = output of function 'getInputData'
//                 - [n_min, {beta_min, beta_max}] = output of function 'retrieveMonData'
//                 - [] = output of function 'computeParams'
//                 - {post_map_integral, pre_map_integral} = output of function
//                                                           'computeQuadGl'
//                 - r = output of function 'computeOrder'
//
//         OUTPUT: no outputs
//
//    DESCRIPTION: once the monomial quadrature rule is completed the resulting data is
//                 streamed in output text files inside the output subdirectory created,
//                 by this routine, within the calling directory of the executable of 
//                 this library. The output data is splitted in three files with
//                 'Results.txt' containing recap informations of the execution 
//                 (including the resulting integral) and 'Nodes.txt' and 'Weights.txt'
//                 containg the classic and new G-L nodes and weights respectively.
//
/////////////////////////////////////////////////////////////////////////////////////////

template<typename type>
void exportNewData(const std::vector<type>& nodes, const std::vector<type>& weights, const std::vector<float128>& output_data)
{
  // Print on-screen quadrature results
  std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
        << " **\n ** I_n(p(x)) = "
        << output_data[0]
        << " **\n ** E_n(p(x)) = " 
        << output_data[1]
        << " **" << std::endl;

  // Write Results.txt file containing transformation review and integral outputs
  std::ofstream Results;
  Results.open("output/Results.txt", std::ios_base::app);
  Results << std::setprecision(std::numeric_limits<float128>::max_digits10)
          << "\n        I_n(f) = "
          << output_data[0]
          << ", E_n(f) = "
          << output_data[1]
          << "  (monomial quadrature rule)"
          << "\n        I_n(f) = "
          << output_data[2]
          << ", E_n(f) = "
          << output_data[3]
          << "  (classical G-L rule)";
  Results.close();

  // Write Nodes.txt file containing transformed G-L nodes using the monomial map
  std::ofstream Nodes;
  Nodes.open("output/Nodes.txt");
  for(int k = 0; k <= nodes.size() - 1; k++)
  {
    Nodes << std::setprecision(std::numeric_limits<type>::max_digits10)
          << nodes[k] << std::endl;
  }
  Nodes.close();

  // Write Weights.txt file containing transformed G-L weights using the monomial map
  std::ofstream Weights;
  Weights.open("output/Weights.txt");
  for(int k = 0; k <= weights.size() - 1; k++)
  {
    Weights << std::setprecision(std::numeric_limits<type>::max_digits10)
            << weights[k] << std::endl;
  }
  Weights.close();
}
template void exportNewData<float128>(const std::vector<float128>& nodes, const std::vector<float128>& weights, const std::vector<float128>& output_data); // Template mock instantiation for non-inline function
template void exportNewData<double>(const std::vector<double>& nodes, const std::vector<double>& weights, const std::vector<float128>& output_data); // Template mock instantiation for non-inline function