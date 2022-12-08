//
// AED, August 2022 (Tomás Oliveira e Silva)
//
// First practical assignement (speed run)
//
// Compile using either
//   cc -Wall -O2 -D_use_zlib_=0 solution_speed_run.c -lm
// or
//   cc -Wall -O2 -D_use_zlib_=1 solution_speed_run.c -lm -lz
//
// Place your student numbers and names here
//   N.Mec. 108122  Name: Alexandre Pedro Ribeiro
//   N.Mec. 110056  Name: Ricardo Manuel Quintaneiro Almeida
//


//
// static configuration
//

#define _max_road_size_  800  // the maximum problem size
#define _min_road_speed_   2  // must not be smaller than 1, shouldnot be smaller than 2
#define _max_road_speed_   9  // must not be larger than 9 (only because of the PDF figure)


//
// include files --- as this is a small project, we include the PDF generation code directly from make_custom_pdf.c
//

#include <math.h>
#include <stdio.h>
#include "../P02/elapsed_time.h"
#include "make_custom_pdf.c"
#include "priorityQueue.h" // required for Solution 6


//
// road stuff
//

static int max_road_speed[1 + _max_road_size_]; // positions 0.._max_road_size_
static int max_road_speed_inverse[1 + _max_road_size_]; // inverse road

static void init_road_speeds(void)
{
  double speed;
  int i;

  for(i = 0;i <= _max_road_size_;i++)
  {
    speed = (double)_max_road_speed_ * (0.55 + 0.30 * sin(0.11 * (double)i) + 0.10 * sin(0.17 * (double)i + 1.0) + 0.15 * sin(0.19 * (double)i));
    max_road_speed[i] = (int)floor(0.5 + speed) + (int)((unsigned int)random() % 3u) - 1;
    if(max_road_speed[i] < _min_road_speed_)
      max_road_speed[i] = _min_road_speed_;
    if(max_road_speed[i] > _max_road_speed_)
      max_road_speed[i] = _max_road_speed_;
  }

  // INVERT THE ROAD

  int n = _max_road_size_ + 1;

	for (i = 0; i < n; i++) {
		max_road_speed_inverse[n - 1 - i] = max_road_speed[i];
	}
	
}


//
// description of a solution
//

typedef struct
{
  int n_moves;                         // the number of moves (the number of positions is one more than the number of moves)
  int positions[1 + _max_road_size_];  // the positions (the first one must be zero)
}
solution_t;


//
// the (very inefficient) recursive solution given to the students
//

static solution_t solution_1,solution_1_best;
static double solution_1_elapsed_time; // time it took to solve the problem
static unsigned long solution_1_count; // effort dispended solving the problem

static void solution_1_recursion(int move_number,int position,int speed,int final_position)
{
  int i,new_speed;

  // record move
  solution_1_count++;
  solution_1.positions[move_number] = position;
  // is it a solution?
  if(position == final_position && speed == 1)
  {
    // is it a better solution?
    if(move_number < solution_1_best.n_moves)
    {
      solution_1_best = solution_1;
      solution_1_best.n_moves = move_number;
    }
    return;
  }
  // no, try all legal speeds
  for(new_speed = speed - 1;new_speed <= speed + 1;new_speed++)
    if(new_speed >= 1 && new_speed <= _max_road_speed_ && position + new_speed <= final_position)
    {
      for(i = 0;i <= new_speed && new_speed <= max_road_speed[position + i];i++)
        ;
      if(i > new_speed)
        solution_1_recursion(move_number + 1,position + new_speed,new_speed,final_position);
    }
}

static void solve_1(int final_position)
{
  if(final_position < 1 || final_position > _max_road_size_)
  {
    fprintf(stderr,"solve_1: bad final_position\n");
    exit(1);
  }
  solution_1_elapsed_time = cpu_time();
  solution_1_count = 0ul;
  solution_1_best.n_moves = final_position + 100;
  solution_1_recursion(0,0,0,final_position);
  solution_1_elapsed_time = cpu_time() - solution_1_elapsed_time;
}


//
// SOLUTION 2
//


static solution_t solution_2,solution_2_best;
static double solution_2_elapsed_time; // time it took to solve the problem
static unsigned long solution_2_count; // effort dispended solving the problem

static void solution_2_recursion(int move_number,int position,int speed,int final_position)
{

  int i,new_speed;

  // record move
  solution_2_count++;
  solution_2.positions[move_number] = position;
  // is it a solution?
  if(position == final_position && speed == 1)
  {
    solution_2_best = solution_2;
    solution_2_best.n_moves = move_number;
    return;
  }
  // if (solution_2.positions[move_number] < solution_2_best.positions[move_number]) return;
  // no, try all legal speeds
  for(new_speed = speed + 1;new_speed >= speed - 1;new_speed--)
  {
    if(new_speed >= 1 && new_speed <= _max_road_speed_ && position + new_speed*(new_speed+1)/2 <= final_position)
    {
      for(i = 0;i <= new_speed && new_speed <= max_road_speed[position + i];i++)
        ;
      if(i > new_speed)
      {
        if ((move_number + 1) >= solution_2_best.n_moves) return;
        solution_2_recursion(move_number + 1,position + new_speed,new_speed,final_position);
      }
    }
  }

}

static void solve_2(int final_position)
{
  if(final_position < 1 || final_position > _max_road_size_)
  {
    fprintf(stderr,"solve_2: bad final_position\n");
    exit(1);
  }
  solution_2_elapsed_time = cpu_time();
  solution_2_count = 0ul;
  solution_2_best.n_moves = final_position + 100;
  solution_2_recursion(0,0,0,final_position);
  solution_2_elapsed_time = cpu_time() - solution_2_elapsed_time;
}

//
// This is a better recursive solution, that instead of starting from the beginning every time it hits a wall, it takes a step back and tries a different path
// The algorithm then starts going 1 by 1, finding out the best possible move for each step of the way
// Any time the algorithm finds a wrong path, it traces back to the source of the problem and takes another direction
//

static solution_t solution_3_best;
static double solution_3_elapsed_time; // time it took to solve the problem
static unsigned long solution_3_count; // effort dispended solving the problem

static int solution_3_test(int move_number, int position, int speed, int final_position)
{
  // register count
  int new_speed, k;
  solution_3_count++;

  // check if it is the solution
  if(position == final_position && speed == 1) {
      solution_3_best.positions[move_number] = position;
      return 0;
  }

  // test possible speeds, starting from fastest one
  for(new_speed = speed + 1;new_speed >= speed - 1;new_speed--){
        if (new_speed >= 1 && new_speed <= _max_road_speed_ && position + new_speed*(new_speed+1)/2 <= final_position) {
          for (k=0;k<=new_speed && new_speed <= max_road_speed[position + k];k++) {
              ;
          }
          if (k > new_speed) {
              if (solution_3_test(move_number + 1,position + new_speed,new_speed,final_position) == 0){
                  solution_3_best.n_moves++;
                  solution_3_best.positions[move_number] = position;
                  return 0;
              }
          }
      }
  }
  return 1;
}

static void solve_3(int final_position)
{
    if(final_position < 1 || final_position > _max_road_size_)
    {
        fprintf(stderr,"solve_3: bad final_position\n");
        exit(1);
    }
    solution_3_best.n_moves = 0ul;
    solution_3_elapsed_time = cpu_time();
    solution_3_count = 0ul;
    solution_3_test(0,0,0,final_position);
    solution_3_elapsed_time = cpu_time() - solution_3_elapsed_time;
}

// Solution 4 it's Solution 3 but with no recursion

static solution_t solution_4_best;
static double solution_4_elapsed_time; // time it took to solve the problem
static unsigned long solution_4_count; // effort dispended solving the problem

static void solution_4_no_recursion(int move_number, int position, int speed, int final_position)
{
    int new_speed, returned, i, k;
    // declare arrays to store move data
    int positions[final_position];
    int speeds[final_position];

    // register count
    solution_4_count++;

    // record move
    positions[0] = position;
    speeds[0] = speed;

    for (i = 0; i <= final_position; i++)
    {
      returned = 1;
      if (positions[i] == final_position && speeds[i] == 1)
      {
        solution_4_best.n_moves = i;
        for (int j = 0; j <= i; j++)
          solution_4_best.positions[j] = positions[j];
        break;
      }
      for (new_speed = speeds[i] + 1; new_speed >= speeds[i] - 1; new_speed--)
      {
        if (new_speed >= 1 && new_speed <= _max_road_speed_ && positions[i] + new_speed * (new_speed + 1) / 2 <= final_position)
        {
          for (k = 0; k <= new_speed && new_speed <= max_road_speed[positions[i] + k]; k++)
            ;
          if (k > new_speed)
          {
            solution_4_count++;
            speeds[i + 1] = new_speed;
            positions[i + 1] = positions[i] + new_speed;
            returned = 0;
            break;
          }
        } 
      }
      for (k = 0; k < i; k++)
      {
        if (returned == 1 && speeds[i - 1] >= 2 && speeds[i - k] >= speeds[i - k - 1])
        {
          solution_4_count++;
          speeds[i - k] = speeds[i - k] - 1;
          positions[i - k] = positions[i - k] - 1;
          i = i - k - 1;
          break;
        }
      }
    }
}

static void solve_4(int final_position)
{
    if(final_position < 1 || final_position > _max_road_size_)
    {
        fprintf(stderr,"solve_4: bad final_position\n");
        exit(1);
    }
    solution_4_best.n_moves = 0ul;
    solution_4_elapsed_time = cpu_time();
    solution_4_count = 0ul;
    solution_4_no_recursion(0,0,0,final_position);
    solution_4_elapsed_time = cpu_time() - solution_4_elapsed_time;
}

//
// SOLUTION 5
//

static solution_t solution_5_best;
static double solution_5_elapsed_time; // time it took to solve the problem
static unsigned long solution_5_count; // effort dispended solving the problem

static int node[_max_road_size_ + 1][3];

#define node_cost 0
#define node_lastPosition 1
#define node_speed 2


static int solution_5_recursion(int position, int final_position)
{
    int i, new_speed;
    int cost = node[position][node_cost];
    int speed = node[position][node_speed];
    solution_5_count++;
    if (position == 0)
    {
        for (int i = 1; i <= final_position; i++)
        {
            node[i][node_cost] = i;
            node[i][node_lastPosition] = i - 1;
            node[i][node_speed] = 1;
        }
        solution_5_recursion(1, final_position);
        return 0;
    }

    if (position == final_position && speed == 1)
    {
        int positions[final_position + 1];
        solution_5_best.n_moves = 0;
        while (position != 0)
        {
          positions[solution_5_best.n_moves] = position;
          position = node[position][node_lastPosition];
          solution_5_best.n_moves++;
        }
        int m = 0;
        for (int j = solution_5_best.n_moves; j > 0; j--)
        {
            solution_5_best.positions[j] = positions[m++];
        }
        return 0;
    }

    int new_position, temp_cost, temp_pos, temp_ns;

    for (new_speed = speed + 1; new_speed >= speed - 1; new_speed--)
    {
        if (new_speed >= 1 && new_speed <= _max_road_speed_ && position + new_speed * (new_speed + 1) / 2 <= final_position)
        {
          new_position = position + new_speed;
          for (i = 0; i <= new_speed && new_speed <= max_road_speed[position + i]; i++)
            ;
          if (i > new_speed)
          {
            temp_cost = node[new_position][node_cost];
            temp_pos = node[new_position][node_lastPosition];
            temp_ns = node[new_position][node_speed];
            if (temp_cost > cost + 1)
            {
                node[new_position][node_cost] = cost + 1;
                node[new_position][node_lastPosition] = position;
                node[new_position][node_speed] = new_speed;
            }
            if (solution_5_recursion(position + 1, final_position) == 0)
                return 0;
            else
            {
              node[new_position][node_cost] = temp_cost;
              node[new_position][node_lastPosition] = temp_pos;
              node[new_position][node_speed] = temp_ns;
            }
          }
        }
    }
    for (i = position + 1; i <= final_position; i++)
    {
        if (node[i][node_cost] < node[position][node_cost])
          break;
    }
    if (i < final_position + 1)
    {
        if (solution_5_recursion(i, final_position) == 0)
          return 0;
    }
    return 1;
}

static void solve_5(int final_position)
{
  if(final_position < 1 || final_position > _max_road_size_)
  {
    fprintf(stderr,"solve_5: bad final_position\n");
    exit(1);
  }
  solution_5_elapsed_time = cpu_time();
  solution_5_count = 0ul;
  solution_5_best.n_moves = final_position + 100;
  solution_5_recursion(0,final_position);
  solution_5_elapsed_time = cpu_time() - solution_5_elapsed_time;
}




static solution_t solution_6_best;
static double solution_6_elapsed_time; // time it took to solve the problem
static unsigned long solution_6_count; // effort dispended solving the problem
static Node* priorityQueue;


static int solution_6_Astar(int move_number, int position, int speed, Node* priorityQueue, int final_position)
{
  int i, new_speed;
  solution_6_count++;
  
  // record move
  solution_6_best.positions[move_number] = position;
  // is it a solution?
  if(position == final_position && speed == 1)
  {
    solution_6_best.n_moves = move_number;
    return 0;
  }
  
  // no, try all legal speeds 
  int count = 0;
  for(new_speed = speed + 1;new_speed >= speed - 1;new_speed--)
  {
    if(new_speed >=1 && new_speed <= _max_road_speed_ && position + new_speed*(new_speed+1)/2 <= final_position)
    {
      for(i = 0;i <= new_speed && new_speed <= max_road_speed[position + i];i++)
        ;
      if(i > new_speed)
      {
        push(&priorityQueue, position+new_speed, _max_road_speed_+1-new_speed+final_position-position, new_speed);
        count++;
      }
    }
  }
  if (count == 0)
    return 1;
	int a = peek(&priorityQueue);
  int b = peek_Speed(&priorityQueue);
  pop(&priorityQueue);
  while(solution_6_Astar(move_number + 1, a, b, &priorityQueue, final_position) == 1)
  {
    count--;
    if (count == 0) return 1;
    a = peek(&priorityQueue);
    b = peek_Speed(&priorityQueue);
    pop(&priorityQueue);
  }
	return 0;
}

static void solve_6(int final_position)
{
  if(final_position < 1 || final_position > _max_road_size_)
  {
    fprintf(stderr,"solve_6: bad final_position\n");
    exit(1);
  }
  solution_6_elapsed_time = cpu_time();
  solution_6_count = 0ul;
  solution_6_best.n_moves = final_position + 100;
  priorityQueue = newNode(0, final_position+1, 0);
  solution_6_Astar(0,0,0,&priorityQueue,final_position);
  solution_6_elapsed_time = cpu_time() - solution_6_elapsed_time;
}



static solution_t solution_7_best;
static double solution_7_elapsed_time; // time it took to solve the problem
static unsigned long solution_7_count; // effort dispended solving the problem

static int solution_7_test(int move_number, int position, int speed, int final_position)
{
  // register count
  int new_speed, k;
  solution_7_count++;

  // check if it is the solution
  if(position == 0 && speed == 1) {
    solution_7_best.positions[move_number] = position;
    solution_7_best.n_moves = move_number;
    return 1;
  }

  // test possible speeds, starting from fastest one
  for(new_speed = speed + 1;new_speed >= speed - 1;new_speed--){
    if (new_speed >= 1 && new_speed <= _max_road_speed_ && position + new_speed*(new_speed+1)/2 >= 1) {
      for (k=0;k<=new_speed && new_speed <= max_road_speed[position - k];k++) {
        ;
      }
      if (k > new_speed) {
          if (solution_7_test(move_number + 1,position - new_speed,new_speed,final_position) == 1){
              solution_7_best.positions[move_number] = position;
              if (position == final_position)
              {
                int n = solution_7_best.n_moves + 1;
                int aux[_max_road_size_+1], i;

	              for (i = 0; i < n; i++) {
	              	aux[n - 1 - i] = solution_7_best.positions[i];
	              }

	              for (i = 0; i < n; i++) {
	              	solution_7_best.positions[i] = aux[i];
	              }
              }
              return 1;
          }
      }
    }
  }
  return 0;
}

static void solve_7(int final_position)
{
    if(final_position < 1 || final_position > _max_road_size_)
    {
        fprintf(stderr,"solve_7: bad final_position\n");
        exit(1);
    }

    solution_7_best.n_moves = 0ul;
    solution_7_elapsed_time = cpu_time();
    solution_7_count = 0ul;
    solution_7_test(0,final_position,0,final_position);
    solution_7_elapsed_time = cpu_time() - solution_7_elapsed_time;
}


//
// example of the slides
//

static void example(void)
{
  int i,final_position;

  srandom(0xAED2022);
  init_road_speeds();
  final_position = 30;
  solve_1(final_position);
  make_custom_pdf_file("example.pdf",final_position,&max_road_speed[0],solution_1_best.n_moves,&solution_1_best.positions[0],solution_1_elapsed_time,solution_1_count,"Plain recursion");
  printf("mad road speeds:");
  for(i = 0;i <= final_position;i++)
    printf(" %d",max_road_speed[i]);
  printf("\n");
  printf("positions:");
  for(i = 0;i <= solution_1_best.n_moves;i++)
    printf(" %d",solution_1_best.positions[i]);
  printf("\n");
  // solve_3(final_position);
  // make_custom_pdf_file("example.pdf",final_position,&max_road_speed[0],solution_3_best.n_moves,&solution_3_best.positions[0],solution_3_elapsed_time,solution_3_count,"Better recursion");
  // printf("mad road speeds:");
  // for(i = 0;i <= final_position;i++)
  //   printf(" %d",max_road_speed[i]);
  // printf("\n");
  // printf("positions:");
  // for(i = 0;i <= solution_3_best.n_moves;i++)
  //   printf(" %d",solution_3_best.positions[i]);
  // printf("\n");

}


//
// main program
//

int main(int argc,char *argv[argc + 1])
{
# define _time_limit_  3600.0
  int n_mec,final_position,print_this_one;
  char file_name[64];

  // generate the example data
  if(argc == 2 && argv[1][0] == '-' && argv[1][1] == 'e' && argv[1][2] == 'x')
  {
    example();
    return 0;
  }
  // initialization
  n_mec = (argc < 2) ? 0xAED2022 : atoi(argv[1]);
  srandom((unsigned int)n_mec);
  init_road_speeds();
  // run all solution methods for all interesting sizes of the problem
  final_position = 1;
  solution_1_elapsed_time = 0.0;
  solution_2_elapsed_time = 0.0;
  solution_3_elapsed_time = 0.0;
  solution_4_elapsed_time = 0.0;
  solution_5_elapsed_time = 0.0;
  solution_6_elapsed_time = 0.0;
  solution_7_elapsed_time = 0.0;
  printf("    + --- ---------------- --------- +\n");
  printf("    |       Speed-Based A* Recursion |\n");
  printf("--- + --- ---------------- --------- +\n");
  printf("  n | sol            count  cpu time |\n");
  printf("--- + --- ---------------- --------- +\n");
  // printf("    + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
  // printf("    |             Very bad recursion |      Slightly better recursion |                 Best recursion |\n");
  // printf("--- + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
  // printf("  n | sol            count  cpu time | sol            count  cpu time | sol            count  cpu time |\n");
  // printf("--- + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
  // double sol_1_times[800];
  // double sol_2_times[800];
  // double sol_3_times[800];


  // FILE* ptr = fopen("sol_6_roadSize10000_110056.txt", "w");

  while(final_position <= _max_road_size_/* && final_position <= 20*/)
  {
    print_this_one = (final_position == 10 || final_position == 20 || final_position == 50 || final_position == 100
      || final_position == 200 || final_position == 400 || final_position == 800) ? 1 : 0;
    printf("%3d |",final_position);
    // fprintf(ptr,"%3d |",final_position);
    // first solution method (very bad)
    // if(solution_1_elapsed_time < _time_limit_)
    // {
    //   solve_1(final_position);
    //   if(print_this_one != 0)
    //   {
    //     sprintf(file_name,"%03d_1.pdf",final_position);
    //     make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_1_best.n_moves,&solution_1_best.positions[0],solution_1_elapsed_time,solution_1_count,"Very bad recursion");
    //   }
    //   printf(" %3d %16lu %9.3e |",solution_1_best.n_moves,solution_1_count,solution_1_elapsed_time);
    // }
    // else
    // {
    //   solution_1_best.n_moves = -1;
    //   printf("                                |");
    // }
    // second solution method (less bad)
    // // ...
    // if(solution_2_elapsed_time < _time_limit_)
    // {
    //   solve_2(final_position);
    //   // if(print_this_one != 0)
    //   // {
    //   //   sprintf(file_name,"%03d_2.pdf",final_position);
    //   //   make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_2_best.n_moves,&solution_2_best.positions[0],solution_2_elapsed_time,solution_2_count,"Slightly better recursion");
    //   // }
    //   printf(" %3d %16lu %9.3e |",solution_2_best.n_moves,solution_2_count,solution_2_elapsed_time);
    // }
    // else
    // {
    //   solution_2_best.n_moves = -1;
    //   printf("                                |");
    // }
    // // done
    // if(solution_3_elapsed_time < _time_limit_)
    // {
    //   solve_3(final_position);
    //   // if(print_this_one != 0)
    //   // {
    //   //   sprintf(file_name,"%03d_3.pdf",final_position);
    //   //   make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_3_best.n_moves,&solution_3_best.positions[0],solution_3_elapsed_time,solution_3_count,"Fast recursion");
    //   // }
    //   printf(" %3d %16lu %9.3e |",solution_3_best.n_moves,solution_3_count,solution_3_elapsed_time);
    //   // fprintf(ptr," %3d %16lu %9.3e |",solution_3_best.n_moves,solution_3_count,solution_3_elapsed_time);
    // }
    // else
    // {
    //   solution_3_best.n_moves = -1;
    //   printf("                                |");
    // }
    // if(solution_4_elapsed_time < _time_limit_)
    // {
    //   solve_4(final_position);
    //   // if(print_this_one != 0)
    //   // {
    //   //   sprintf(file_name,"%03d_4_%d.pdf",final_position,n_mec);
    //   //   make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_4_best.n_moves,&solution_4_best.positions[0],solution_4_elapsed_time,solution_4_count,"Fast Dynamic Non-Recursion");
    //   // }
    //   printf(" %3d %16lu %9.3e |",solution_4_best.n_moves,solution_4_count,solution_4_elapsed_time);
    //   // fprintf(ptr," %3d %16lu %9.3e |",solution_4_best.n_moves,solution_4_count,solution_4_elapsed_time);

    // }
    // else
    // {
    //   solution_4_best.n_moves = -1;
    //   printf("                                |");
    // }
    // if(solution_5_elapsed_time < _time_limit_)
    // {
    //   solve_5(final_position);
    //   // if(print_this_one != 0)
    //   // {
    //   //   sprintf(file_name,"%03d_5_%d.pdf",final_position,n_mec);
    //   //   make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_5_best.n_moves,&solution_5_best.positions[0],solution_5_elapsed_time,solution_5_count,"Unitary-Cost Dijkstra Recursion");
    //   // }
    //   printf(" %3d %16lu %9.3e |",solution_5_best.n_moves,solution_5_count,solution_5_elapsed_time);
    //   // fprintf(ptr," %3d %16lu %9.3e |",solution_5_best.n_moves,solution_5_count,solution_5_elapsed_time);
    // }
    // else
    // {
    //   solution_5_best.n_moves = -1;
    //   printf("                                |");
    // }
    if(solution_6_elapsed_time < _time_limit_)
    {
      solve_6(final_position);
      // if(print_this_one != 0)
      // {
      //   sprintf(file_name,"%03d_6_%d.pdf",final_position,n_mec);
      //   make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_6_best.n_moves,&solution_6_best.positions[0],solution_6_elapsed_time,solution_6_count,"Speed-Based A* Recursion");
      // }
      printf(" %3d %16lu %9.3e |",solution_6_best.n_moves,solution_6_count,solution_6_elapsed_time);
      // fprintf(ptr," %3d %16lu %9.3e |",solution_6_best.n_moves,solution_6_count,solution_6_elapsed_time);
    }
    else
    {
      solution_6_best.n_moves = -1;
      printf("                                |");
    }
    // if(solution_7_elapsed_time < _time_limit_)
    // {
    //   solve_7(final_position);
    //   if(print_this_one != 0)
    //   {
    //     sprintf(file_name,"%03d_7.pdf",final_position);
    //     make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_7_best.n_moves,&solution_7_best.positions[0],solution_7_elapsed_time,solution_7_count,"Best recursion (reverse)");
    //   }
    //   printf(" %3d %16lu %9.3e |",solution_7_best.n_moves,solution_7_count,solution_7_elapsed_time);
    // }
    // else
    // {
    //   solution_7_best.n_moves = -1;
    //   printf("                                |");
    // }
    // if(solution_8_elapsed_time < _time_limit_)
    // {
    //   solve_8(final_position);
    //   // if(print_this_one != 0)
    //   // {
    //   //   sprintf(file_name,"%03d_3.pdf",final_position);
    //   //   make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_3_best.n_moves,&solution_3_best.positions[0],solution_3_elapsed_time,solution_3_count,"Best recursion");
    //   // }
    //   printf(" %3d %16lu %9.3e |",solution_8_best.n_moves,solution_8_count,solution_8_elapsed_time);
    // }
    // else
    // {
    //   solution_8_best.n_moves = -1;
    //   printf("                                |");
    // }
    printf("\n");
    // fprintf(ptr,"\n");
    // if (solution_3_best.n_moves != solution_5_best.n_moves ) {
    //   printf("ERRO NO 3º ou 5º ALGORITMO!\n");
    //   return EXIT_FAILURE;
    // }
    fflush(stdout);
    // fflush(ptr);
    // new final_position
    if(final_position < 50)
      final_position += 1;
    else if(final_position < 100)
      final_position += 5;
    else if(final_position < 200)
      final_position += 10;
    else
      final_position += 20;
  }
  // fclose(ptr);
  // // printf("--- + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
  printf("--- + --- ---------------- --------- +\n");
  // printf("\nSolution 1 times:\n");
  // for (int i = 0; i < 800; i++) {
  //   printf("%.9lf ",sol_1_times[i]);
  // }
  // printf("\n\nSolution 2 times:\n\n");
  // for (int i = 0; i < 800; i++) {
  //   printf("%.9lf ",sol_2_times[i]);
  // }
  // printf("\n\nSolution 3 times:\n\n");
  // for (int i = 0; i < 800; i++) {
  //   printf("%.9lf ",sol_3_times[i]);
  // }
  return 0;
# undef _time_limit_
}
