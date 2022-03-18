#include "base/base_inc.h"
#include "os/os_inc.h"
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

#include <stdio.h>

////////////////////////////////
// NOTE(allen): Types

struct Array_U32{
  U32 *v;
  U64 count;
};

struct Array_U8{
  U8 *v;
  U64 count;
};

struct EulerData{
  U8 *v;
  U64 count;
  U64 line_count;
};

struct ListNode_U32{
  ListNode_U32 *next;
  Array_U32 array;
};

struct List_U32{
  ListNode_U32 *first;
  ListNode_U32 *last;
  U64 node_count;
  U64 total_count;
};

struct Pair_U64{
  U64 a;
  U64 b;
};

struct Pair_U32{
  U32 a;
  U32 b;
};

struct U64x3{
  U64 v[3];
};

struct U64x3_DivR{
  U64x3 q;
  U64 r;
};

////////////////////////////////
// NOTE(allen): Assembly Function Declarations

c_linkage U64 triangle_number(U64 n);
c_linkage U64 sum_of_squares(U64 n);

c_linkage void fibonacci_stepper_mul(void *dst, void *src);
c_linkage void fibonacci_stepper_sqr(void *dst);
c_linkage U64 fibonacci_number(U64 n);
c_linkage U64 fibonacci_sigma(U64 n);

c_linkage U32 prime_sieve__asm(B8 *table_memory,
                               U32 *primes_list,
                               U32 first, U32 opl,
                               ListNode_U32 *node);

c_linkage void factorization_table__asm(U32 *table_memory, U32 count);

c_linkage U32  get_prime_factor(U32 *fact_tbl, U32 count, U32 n);
c_linkage Pair_U32 max_divide(U32 n, U32 f);
c_linkage U32  divisor_count(U32 *fact_tbl, U32 count, U32 n);

c_linkage Pair_U64 find_bounded_factors(U64 min, U64 max, U64 target_product);

c_linkage U64 gcd_euclidean(U64 a, U64 b);
c_linkage U64 lcm_euclidean(U64 a, U64 b);

c_linkage EulerData euler_data_from_text__asm(String8 text, U8 *memory);
c_linkage EulerData euler_data_from_text_2dig__asm(String8 text, U8 *memory);

c_linkage U64x3      add_u64x3(U64x3 a, U64x3 b);
c_linkage U64x3      add_small_u64x3(U64x3 a, U64 b);
c_linkage U64x3      mul_small_u64x3(U64x3 a, U64 b);
c_linkage U64x3_DivR div_small_u64x3(U64x3 n, U64 d);

c_linkage U64x3      u64x3_from_dec(U8 *digits, U64 count);
c_linkage U64        dec_from_u64x3__asm(U8 *out, U64x3 x);

////////////////////////////////
// NOTE(allen): Helpers/Wrappers with C/C++ Niceness

function void
prime_sieve(M_Arena *arena, List_U32 *primes, U32 first, U32 opl){
  Assert(Implies(primes->node_count == 0, first == 3));
  Assert(first < opl);
  
  // allocate memory
  M_ArenaTemp scratch = m_get_scratch(&arena, 1);
  U32 max_count = opl - first;
  U32 *primes_list = push_array(arena, U32, max_count);
  B8  *sieve_table = push_array_zero(scratch.arena, B8, max_count);
  
  // run sieve
  U32 count = prime_sieve__asm(sieve_table, primes_list, first, opl,
                               primes->first);
  
  // put back memory we didn't use in the primes list
  m_arena_pop_amount(arena, sizeof(*primes_list)*(max_count - count));
  m_arena_align(arena, 8);
  
  // insert new primes block onto the list
  Array_U32 array = {};
  array.v = primes_list;
  array.count = count;
  
  ListNode_U32 *node = push_array(arena, ListNode_U32, 1);
  node->array = array;
  SLLQueuePush(primes->first, primes->last, node);
  primes->node_count += 1;
  primes->total_count += array.count;
  
  m_end_temp(scratch);
}

function Array_U32
factorization_table(M_Arena *arena, U32 count){
  U32 *tbl = push_array_zero(arena, U32, count);
  factorization_table__asm(tbl, count);
  
  Array_U32 result = {};
  result.v = tbl;
  result.count = count;
  
  return(result);
}

function EulerData
euler_data_from_text(M_Arena *arena, String8 text, U64 num_digits){
  Assert(num_digits == 1 || num_digits == 2);
  
  U8 *memory = push_array(arena, U8, text.size);
  EulerData result = {};
  if (num_digits == 1){
    result = euler_data_from_text__asm(text, memory);
  }
  else if (num_digits == 2){
    result = euler_data_from_text_2dig__asm(text, memory);
  }
  m_arena_pop_amount(arena, text.size - result.count);
  m_arena_align(arena, 8);
  
  return(result);
}

c_linkage Array_U8
dec_from_u64x3(M_Arena *arena, U64x3 x){
  Array_U8 result = {};
  
  if (x.v[0] == 0 && x.v[1] == 0 && x.v[2] == 0){
    local U8 zbuffer[1] = {0};
    result.v = zbuffer;
    result.count = 1;
  }
  else{
    U64 cap = 60;
    
    U8 *buffer = push_array(arena, U8, cap);
    U64 count = dec_from_u64x3__asm(buffer, x);
    m_arena_pop_amount(arena, cap - count);
    m_arena_align(arena, 8);
    
    result.v = buffer;
    result.count = count;
  }
  
  return(result);
}

char euler13_text[] =
"37107287533902102798797998220837590246510135740250\n"
"46376937677490009712648124896970078050417018260538\n"
"74324986199524741059474233309513058123726617309629\n"
"91942213363574161572522430563301811072406154908250\n"
"23067588207539346171171980310421047513778063246676\n"
"89261670696623633820136378418383684178734361726757\n"
"28112879812849979408065481931592621691275889832738\n"
"44274228917432520321923589422876796487670272189318\n"
"47451445736001306439091167216856844588711603153276\n"
"70386486105843025439939619828917593665686757934951\n"
"62176457141856560629502157223196586755079324193331\n"
"64906352462741904929101432445813822663347944758178\n"
"92575867718337217661963751590579239728245598838407\n"
"58203565325359399008402633568948830189458628227828\n"
"80181199384826282014278194139940567587151170094390\n"
"35398664372827112653829987240784473053190104293586\n"
"86515506006295864861532075273371959191420517255829\n"
"71693888707715466499115593487603532921714970056938\n"
"54370070576826684624621495650076471787294438377604\n"
"53282654108756828443191190634694037855217779295145\n"
"36123272525000296071075082563815656710885258350721\n"
"45876576172410976447339110607218265236877223636045\n"
"17423706905851860660448207621209813287860733969412\n"
"81142660418086830619328460811191061556940512689692\n"
"51934325451728388641918047049293215058642563049483\n"
"62467221648435076201727918039944693004732956340691\n"
"15732444386908125794514089057706229429197107928209\n"
"55037687525678773091862540744969844508330393682126\n"
"18336384825330154686196124348767681297534375946515\n"
"80386287592878490201521685554828717201219257766954\n"
"78182833757993103614740356856449095527097864797581\n"
"16726320100436897842553539920931837441497806860984\n"
"48403098129077791799088218795327364475675590848030\n"
"87086987551392711854517078544161852424320693150332\n"
"59959406895756536782107074926966537676326235447210\n"
"69793950679652694742597709739166693763042633987085\n"
"41052684708299085211399427365734116182760315001271\n"
"65378607361501080857009149939512557028198746004375\n"
"35829035317434717326932123578154982629742552737307\n"
"94953759765105305946966067683156574377167401875275\n"
"88902802571733229619176668713819931811048770190271\n"
"25267680276078003013678680992525463401061632866526\n"
"36270218540497705585629946580636237993140746255962\n"
"24074486908231174977792365466257246923322810917141\n"
"91430288197103288597806669760892938638285025333403\n"
"34413065578016127815921815005561868836468420090470\n"
"23053081172816430487623791969842487255036638784583\n"
"11487696932154902810424020138335124462181441773470\n"
"63783299490636259666498587618221225225512486764533\n"
"67720186971698544312419572409913959008952310058822\n"
"95548255300263520781532296796249481641953868218774\n"
"76085327132285723110424803456124867697064507995236\n"
"37774242535411291684276865538926205024910326572967\n"
"23701913275725675285653248258265463092207058596522\n"
"29798860272258331913126375147341994889534765745501\n"
"18495701454879288984856827726077713721403798879715\n"
"38298203783031473527721580348144513491373226651381\n"
"34829543829199918180278916522431027392251122869539\n"
"40957953066405232632538044100059654939159879593635\n"
"29746152185502371307642255121183693803580388584903\n"
"41698116222072977186158236678424689157993532961922\n"
"62467957194401269043877107275048102390895523597457\n"
"23189706772547915061505504953922979530901129967519\n"
"86188088225875314529584099251203829009407770775672\n"
"11306739708304724483816533873502340845647058077308\n"
"82959174767140363198008187129011875491310547126581\n"
"97623331044818386269515456334926366572897563400500\n"
"42846280183517070527831839425882145521227251250327\n"
"55121603546981200581762165212827652751691296897789\n"
"32238195734329339946437501907836945765883352399886\n"
"75506164965184775180738168837861091527357929701337\n"
"62177842752192623401942399639168044983993173312731\n"
"32924185707147349566916674687634660915035914677504\n"
"99518671430235219628894890102423325116913619626622\n"
"73267460800591547471830798392868535206946944540724\n"
"76841822524674417161514036427982273348055556214818\n"
"97142617910342598647204516893989422179826088076852\n"
"87783646182799346313767754307809363333018982642090\n"
"10848802521674670883215120185883543223812876952786\n"
"71329612474782464538636993009049310363619763878039\n"
"62184073572399794223406235393808339651327408011116\n"
"66627891981488087797941876876144230030984490851411\n"
"60661826293682836764744779239180335110989069790714\n"
"85786944089552990653640447425576083659976645795096\n"
"66024396409905389607120198219976047599490197230297\n"
"64913982680032973156037120041377903785566085089252\n"
"16730939319872750275468906903707539413042652315011\n"
"94809377245048795150954100921645863754710598436791\n"
"78639167021187492431995700641917969777599028300699\n"
"15368713711936614952811305876380278410754449733078\n"
"40789923115535562561142322423255033685442488917353\n"
"44889911501440648020369068063960672322193204149535\n"
"41503128880339536053299340368006977710650566631954\n"
"81234880673210146739058568557934581403627822703280\n"
"82616570773948327592232845941706525094512325230608\n"
"22918802058777319719839450180888072429661980811197\n"
"77158542502016545090413245809786882778948721859617\n"
"72107838435069186155435662884062257473692284509516\n"
"20849603980134001723930671666823555245252804609722\n"
"53503534226472524250874054075591789781264330331690\n";

int
main(void){
  M_Arena *arena = m_alloc_arena();
  
  String8 text0 = str8_lit(euler13_text);
  EulerData data = euler_data_from_text(arena, text0, 1);
  
  U64x3 x = {0};
  for (U64 i = 0; i < data.line_count; i += 1){
    U64x3 c = u64x3_from_dec(data.v + 50*i, 50);
    x = add_u64x3(x, c);
  }
  
  Array_U8 x_dec = dec_from_u64x3(arena, x);
  for (U64 i = 0; i < 10; i += 1){
    fprintf(stdout, "%u", x_dec.v[i]);
  }
  fprintf(stdout, "\n");
  
  return(0);
}
