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
//   N.Mec. XXXXXX  Name: XXXXXXX
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


//
// road stuff
//

static int max_road_speed[1 + _max_road_size_]; // positions 0.._max_road_size_

static void init_road_speeds(void)
{
  double speed;
  int i;

  for(i = 0;i <= _max_road_size_;i++)
  {
    speed = (double)_max_road_speed_ * (0.55 + 0.30 * sin(0.11 * (double)i) + 0.10 * sin(0.17 * (double)i + 1.0) + 0.15 * sin(0.19 * (double)i));
    // speed = (double)_max_road_speed_ * (0.55 + 0.30 * sin(0.2 * (double)i) + 0.10 * sin(0.3 * (double)i + 1.0) + 0.15 * sin(0.4 * (double)i));
    max_road_speed[i] = (int)floor(0.5 + speed) + (int)((unsigned int)random() % 3u) - 1;
    if(max_road_speed[i] < _min_road_speed_)
      max_road_speed[i] = _min_road_speed_;
    if(max_road_speed[i] > _max_road_speed_)
      max_road_speed[i] = _max_road_speed_;
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
        return 1;
    }

    // test possible speeds, starting from fastest one
    for(new_speed = speed + 1;new_speed >= speed - 1;new_speed--){
        if (new_speed >= 1 && new_speed <= _max_road_speed_ && position + new_speed*(new_speed+1)/2 <= final_position && position + new_speed <= final_position) {
            for (k=0;k<=new_speed && new_speed <= max_road_speed[position + k];k++) {
                ;
            }
            if (k > new_speed) {
                if (solution_3_test(move_number + 1,position + new_speed,new_speed,final_position) == 1){
                    solution_3_best.n_moves++;
                    solution_3_best.positions[move_number] = position;
                    return 1;
                }
            }
        }
    }
    return 0;
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
    int positions[final_position];
    int speeds[final_position];

    // register count
    int new_speed, returned, i, j, k;
    solution_4_count++;

    positions[0] = position;
    speeds[0] = speed;

    for (i = 0; i <= final_position; i++)
    {
      returned = 0;
      if (positions[i] == final_position && speeds[i] == 1)
      {
        solution_4_best.n_moves = i;
        for (j = 0; j <= i; j++)
          {solution_4_best.positions[j] = positions[j];}
        break;
      }
      for (new_speed = speeds[i] + 1; new_speed >= speeds[i] - 1; new_speed--)
      {
        if (new_speed >= 1 && new_speed <= _max_road_speed_ && positions[i] + new_speed * (new_speed + 1) / 2 <= final_position && positions[i] + new_speed <= final_position)
        {
          for (k = 0; k <= new_speed && new_speed <= max_road_speed[positions[i] + k]; k++)
          {
            ;
          }
          if (k > new_speed)
          {
            solution_4_count++;
            speeds[i + 1] = new_speed;
            positions[i + 1] = positions[i] + new_speed;
            returned = 1;
            break;
          }
        } 
      }
      for (k = 0; k < i; k++)
      {
        if (returned == 0 && speeds[i - 1] >= 2 && speeds[i - k] >= speeds[i - k - 1])
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



static solution_t solution_5_best;
static int vertex[_max_road_size_+1][3];
static double solution_5_elapsed_time; // time it took to solve the problem
static unsigned long solution_5_count; // effort dispended solving the problem
static int debug;

static int solution_5_recursion(int position, int final_position)
{
  int i, new_speed;
  int cost = vertex[position][0];
  int speed = vertex[position][2];
  solution_5_count++;
  if (position == 0)
  {
    debug = 0;
    for (int i = 1; i <= final_position; i++)
    {
      vertex[i][0] = i;
      vertex[i][1] = i - 1;
      vertex[i][2] = 1;
    }
    solution_5_recursion(1, final_position);
    return 1;
  }

  if (position == final_position && speed == 1)
  {
    int positions[final_position + 1];
    solution_5_best.n_moves = 0;
    while (position != 0)
    {
      positions[solution_5_best.n_moves] = position;
      position = vertex[position][1];
      solution_5_best.n_moves++;
    }
    int m = 0;
    for (int j = solution_5_best.n_moves; j > 0; j--)
    {
      solution_5_best.positions[j] = positions[m++];
    }
    return 1;
  }
  if (position == final_position && speed != 1)     // SAFE
    {printf("For position: %d",position);
    return 0;}
  int temp_cost,temp_pos,temp_ns; // SAFE
  for (new_speed = speed + 1; new_speed >= speed - 1; new_speed--)
  {
    if (new_speed >= 1 && new_speed <= _max_road_speed_ && position + new_speed * (new_speed + 1) / 2 <= final_position)
    {
      for (i = 0; i <= new_speed && new_speed <= max_road_speed[position + i]; i++)
        ;
      if (i > new_speed)
      {
        if (position == 378 && cost < 378){
          printf("New speed: %d",new_speed);
        }
        temp_cost = vertex[position + new_speed][0];
        temp_pos = vertex[position + new_speed][1];
        temp_ns = vertex[position + new_speed][2];
        if (temp_cost > cost + 1)
        {
          if (position == 378 && new_speed == 2 && cost<378) puts("check");
          vertex[position + new_speed][0] = cost + 1;
          vertex[position + new_speed][1] = position;
          vertex[position + new_speed][2] = new_speed;
          if (position == 377 && debug == 13)
          {
            for (int d = position; d<=final_position;d++)
            {
              printf("Position: %d --- Cost: %d --- Speed: %d --- Last: %d\n",
                d,vertex[d][0],vertex[d][2],vertex[d][1]);
            }
          }
        }
        // else if (temp_cost == cost + 1)
        // {
        //   if (temp_ns < new_speed)
        //   {
        //     vertex[position + new_speed][1] = position;           // SAFE
        //     vertex[position + new_speed][2] = new_speed;
        //   }
        // }
        if (solution_5_recursion(position + 1, final_position) == 1)
          return 1;
        else
        {

          vertex[position + new_speed][0] = temp_cost;
          vertex[position + new_speed][1] = temp_pos;             // SAFE
          vertex[position + new_speed][2] = temp_ns;
        }
      }
    }
  }
  return 0;
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
  make_custom_pdf_file("example.pdf",final_position,&max_road_speed[0],solution_1_best.n_moves,&solution_1_best.positions[0],solution_1_elapsed_time,solution_1_count,"Slightly better recursion");
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
  // printf("    + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
  // printf("    |             Very bad recursion |      Slightly better recursion |                 Best recursion |\n");
  // printf("--- + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
  // printf("  n | sol            count  cpu time | sol            count  cpu time | sol            count  cpu time |\n");
  // printf("--- + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
  // double sol_1_times[800];
  // double sol_2_times[800];
  // double sol_3_times[800];
  while(final_position <= _max_road_size_/* && final_position <= 20*/)
  {
    print_this_one = (final_position == 10 || final_position == 20 || final_position == 50 || final_position == 100 || final_position == 200 || final_position == 400 || final_position == 800 || final_position == 551 || final_position == 385) ? 1 : 0;
    printf("%3d |",final_position);
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
    //   if(print_this_one != 0)
    //   {
    //     sprintf(file_name,"%03d_2.pdf",final_position);
    //     make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_2_best.n_moves,&solution_2_best.positions[0],solution_2_elapsed_time,solution_2_count,"Slightly better recursion");
    //   }
    //   printf(" %3d %16lu %9.3e |",solution_2_best.n_moves,solution_2_count,solution_2_elapsed_time);
    // }
    // else
    // {
    //   solution_2_best.n_moves = -1;
    //   printf("                                |");
    // }
    // // done
    if(solution_3_elapsed_time < _time_limit_)
    {
      solve_3(final_position);
      if(print_this_one != 0)
      {
        sprintf(file_name,"%03d_3.pdf",final_position);
        make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_3_best.n_moves,&solution_3_best.positions[0],solution_3_elapsed_time,solution_3_count,"Best recursion");
      }
      printf(" %3d %16lu %9.3e |",solution_3_best.n_moves,solution_3_count,solution_3_elapsed_time);
    }
    else
    {
      solution_3_best.n_moves = -1;
      printf("                                |");
    }
    // if(solution_4_elapsed_time < _time_limit_)
    // {
    //   solve_4(final_position);
    //   // if(print_this_one != 0)
    //   // {
    //   //   sprintf(file_name,"%03d_4.pdf",final_position);
    //   //   make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_4_best.n_moves,&solution_4_best.positions[0],solution_4_elapsed_time,solution_4_count,"No recursion");
    //   // }
    //   printf(" %3d %16lu %9.3e |",solution_4_best.n_moves,solution_4_count,solution_4_elapsed_time);
    // }
    // else
    // {
    //   solution_4_best.n_moves = -1;
    //   printf("                                |");
    // // }
    if(solution_5_elapsed_time < _time_limit_)
    {
      solve_5(final_position);
      if(print_this_one != 0)
      {
        sprintf(file_name,"%03d_5.pdf",final_position);
        make_custom_pdf_file(file_name,final_position,&max_road_speed[0],solution_5_best.n_moves,&solution_5_best.positions[0],solution_5_elapsed_time,solution_5_count,"Dijkstra recursion");
      }
      printf(" %3d %16lu %9.3e |",solution_5_best.n_moves,solution_5_count,solution_5_elapsed_time);
    }
    else
    {
      solution_5_best.n_moves = -1;
      printf("                                |");
    }
    printf("\n");
    if (solution_3_best.n_moves != solution_5_best.n_moves ) {
      printf("ERRO NO 3º ou 5º ALGORITMO!\n");
      return EXIT_FAILURE;
    }
    fflush(stdout);
    // new final_position
    if(final_position < 5000)
      final_position += 1;
    else if(final_position < 100)
      final_position += 5;
    else if(final_position < 200)
      final_position += 10;
    else
      final_position += 20;
  }
  // printf("--- + --- ---------------- --------- + --- ---------------- --------- + --- ---------------- --------- +\n");
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
