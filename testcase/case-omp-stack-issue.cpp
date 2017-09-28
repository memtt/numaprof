void add(int * a,int i)
{
        *a+=i;
}

void run()
{
        int a;
        for (int i = 0 ; i < 1024 ; i++)
                add(&a,i);
}

int main()
{
        #pragma omp parallel for
        for (int i = 0 ; i < 1024 ; i++)
                run();
}
