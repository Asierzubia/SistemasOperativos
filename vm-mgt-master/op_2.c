/***************************************************
*       版权声明
*
*   本操作系统名为：MINE
*   该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*   只允许个人学习以及公开交流使用
*
*   代码最终所有权及解释权归田宇所有；
*
*   本模块作者：  田宇
*   EMail:      345538255@qq.com
*
*
***************************************************/

#include "memory.h"
#include "lib.h"


unsigned long page_init(struct Page * page,unsigned long flags)
{
    if(!page->attribute)
    {
        /*
            对于一个崭新的页面而言，page_init函数是对页结构进行初始化
        */
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->attribute = flags;
        page->reference_count++;
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page->zone_struct->total_pages_link++;
    }
    else if((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U) || (flags & PG_Referenced) || (flags & PG_K_Share_To_U))
    {
        /*
            如果当前页面结构属性或参数flags中含有引用属性（PG_Referenced）或共享属性（PG_K_Share_To_U），
            那么就只增加struct page结构体的引用计数和struct zone结构体的页面被引用计数
        */
        page->attribute |= flags;
        page->reference_count++;
        page->zone_struct->total_pages_link++;
    }
    else
    {
        /*
            否则就仅仅是添加页表属性，并置位bit映射位图的相应位。
        */
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64; 
        page->attribute |= flags;
    }
    return 0;
}


unsigned long page_clean(struct Page * page)
{
    if(!page->attribute)
    {
        page->attribute = 0;
    }
    else if((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U))
    {       
        page->reference_count--;
        page->zone_struct->total_pages_link--;
        if(!page->reference_count)
        {
            page->attribute = 0;
            page->zone_struct->page_using_count--;
            page->zone_struct->page_free_count++;
        }
    }
    else
    {
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) &= ~(1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64);

        page->attribute = 0;
        page->reference_count = 0;
        page->zone_struct->page_using_count--;
        page->zone_struct->page_free_count++;
        page->zone_struct->total_pages_link--;
    }
    return 0;
}


void init_memory()
{
    int i,j;
    unsigned long TotalMem = 0 ;
    struct E820 *p = NULL;  
    
    color_printk(BLUE,BLACK,"Display Physics Address MAP,Type(1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI NVS Memory,Others:Undefine)\n");
    p = (struct E820 *)0xffff800000007e00;


    /*
        代码清单4-61 获得物理内存信息
    */ 
    for(i = 0;i < 32;i++)
    {
        color_printk(ORANGE,BLACK,"Address:%#018lx\tLength:%#018lx\tType:%#010x\n",p->address,p->length,p->type);
        unsigned long tmp = 0;
        if(p->type == 1)
            TotalMem +=  p->length;

        memory_management_struct.e820[i].address += p->address;

        memory_management_struct.e820[i].length  += p->length;

        memory_management_struct.e820[i].type    = p->type;
        
        memory_management_struct.e820_length = i;

        p++;
        if(p->type > 4 || p->length == 0 || p->type < 1)
            break;      
    }
    
    // 此刻的TotalMem是 9f000h+7fef0000h=7ff8f000h

    color_printk(ORANGE,BLACK,"OS Can Used Total RAM:%#018lx\n",TotalMem);


    /*
    
        代码清单 4-62 计算可用物理页数
        
        Address:0x000000 Length:0x00009f00 Type=0x0001
            start:0x0  end:0x0
        Address:0x100000 Length:0x7fef0000 Type=0x0001
            start:0x200000  end:0x7fe00000
            
        当end<=start时，说明该物理内存段大小<=2MB，即无法分配一个2MB的物理页
        
        计算得知，总的内存可以分配的物理页数是1022（十进制）页，这其中包括了内核程序已经占用的内存
    */


    TotalMem = 0;

    for(i = 0;i <= memory_management_struct.e820_length;i++)
    {
        unsigned long start,end;
        if(memory_management_struct.e820[i].type != 1)
            continue;
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end   = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if(end <= start)
            continue;
        TotalMem += (end - start) >> PAGE_2M_SHIFT;
    }
    
    // 此刻的TotalMem是 对齐后可使用的2MB物理页页数
    color_printk(ORANGE,BLACK,"OS Can Used Total 2M PAGEs:%#010x=%010d\n",TotalMem,TotalMem);


    /*
    
        代码清单 4-68 初始化位图 bits_map
        
    
    */



    // 此刻的TotalMem是 0x fffc 0000 + 0x4 0000 = 0x 1 0000 0000
    // 
    TotalMem = memory_management_struct.e820[memory_management_struct.e820_length].address 
                + memory_management_struct.e820[memory_management_struct.e820_length].length;
    
    //color_printk(WHITE,RED,"List4-68 TotalMem:%#018lx\n",TotalMem);

    
    // bits map construction init
    
    // end_brk : 0x114a08  bits_map:0x115000
    // 从内核程序结束地址 end_brk 起的4KB上边界对齐处 存入一个 bits_map 
    memory_management_struct.bits_map = (unsigned long *)((memory_management_struct.end_brk + PAGE_4K_SIZE-1) & PAGE_4K_MASK);
    
    // bits_size = 0x800 按2MB对齐后物理地址空间可分页数 (800H = 2048D) 
    // 从 Address:0x00开始的全部：可以用物理段、内存空洞以及ROM空间
    // bits_length=0x100 位图长度 (100H = 256D)
    memory_management_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;

    // 8位等于1字节，1位表示一个页
    // bits_length是位图长度，
    memory_management_struct.bits_length = (((unsigned long)(TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & ( ~ (sizeof(long) - 1));

    // 将整个bits_map 全部置1，以标注非内存页(内存空洞和ROM空间)已被使用
    memset(memory_management_struct.bits_map,0xff,memory_management_struct.bits_length);        //init bits map memory

    
    
    
    /*
    
        代码清单 4-69 初始化结构体 struct page
    
    */
    

    // pages construction init
    
    // 在bits_map 之后，找一个4K对齐的地址处开始放 pages_struct
    // bits_map_end:0x115800     pages_struct: 0x116000
    memory_management_struct.pages_struct = (struct Page *)(((unsigned long)memory_management_struct.bits_map 
                                                        + memory_management_struct.bits_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    
    memory_management_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;
    
    memory_management_struct.pages_length = ((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & ( ~ (sizeof(long) - 1));
    
    // 将整个 pages_struct 清零 
    memset(memory_management_struct.pages_struct,0x00,memory_management_struct.pages_length);   //init pages memory

    
    
    
    /*
    
        代码清单 4-70 为 struct zone 结构体建立存储空间并对其进行初始化
    
    */

    //zones construction init
    // 在 struct page之后 4K边界对齐的地址处放 struct zone
    // pages_struct_end:0x12a000     zones_struct: 0x12a000
    memory_management_struct.zones_struct = (struct Zone *)(((unsigned long)memory_management_struct.pages_struct 
                                                            + memory_management_struct.pages_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    // 是说有多少个Zone
    memory_management_struct.zones_size   = 0;
    
    // 是说结构体 struct Zone 数组的长度 暂且算作5个
    memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));



    /*
    
        代码清单 4-71 
    
    */  

    for(i = 0;i <= memory_management_struct.e820_length;i++)
    {
        
        unsigned long start,end;
        struct Zone * z;
        struct Page * p;
        unsigned long * b;


        // 找到一个可用的物理内存段
        // 现在只有一个物理内存段可用( TYPE=0x01 且 大小 >2MB )  
        //      Address:0x100000 Length:0x7fef0000 Type=0x0001
        //          start:0x200000  end:0x7fe00000

        if(memory_management_struct.e820[i].type != 1)
            continue;
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end   = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if(end <= start)
            continue;
        
        //zone init
        // 一开始这个zones_size是0 同时表示这是第一个可用的zone
        //color_printk(WHITE,RED,"List4-71  memory_management_struct.zones_size:%#018lx\n",memory_management_struct.zones_size);
        z = memory_management_struct.zones_struct + memory_management_struct.zones_size;
        memory_management_struct.zones_size++;



        // start:0x200000
        z->zone_start_address = start;
        z->zone_end_address = end;
        z->zone_length = end - start;

        z->page_using_count = 0;
        z->page_free_count = (end - start) >> PAGE_2M_SHIFT;

        z->total_pages_link = 0;

        z->attribute = 0;
        z->GMD_struct = &memory_management_struct;

        // 本区域包含的struct Page 结构体数量 
        // 一个可以用的物理页由一个 struct page 结构体管理
        // 这里已经是只需要 type=0x01以及>2MB的 可用物理段 了
        // 本质上是从物理地址 0x200000 开始的内存
        // （也就是说除掉内核程序占用的一部分内存，还有1022个2MB空闲可用物理页）
        // 这里z->pages_length的数值就是 0x3fe=1022
        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        
        // bits_map_end:0x115800     pages_struct: 0x116000
        // z->pages_group 是struct Page 结构体数组指针
        // pages_struct++的话 起始是加 40字节 40D=28H 是一个struct Page 结构占用的字节数
        z->pages_group =  (struct Page *)(memory_management_struct.pages_struct + (start >> PAGE_2M_SHIFT));


        // 由于这里 (start >> PAGE_2M_SHIFT) 计算得到1
        // 同时 struct page 结构体大小是 40字节 40D=28H
        // 所以在输出截图中 z->pages_group 显示是 0x116028
        //color_printk(WHITE,RED,"List4-71  start:%#018lx \t start >> PAGE_2M_SHIFT:%#018lx\n",start, start >> PAGE_2M_SHIFT);
        //color_printk(WHITE,RED,"List4-71  z->pages_length:%#018lx\n",z->pages_length);
        //color_printk(WHITE,RED,"List4-71  z->pages_group:%#018lx\n",z->pages_group);


        //page init
        //为区域内的每个物理页 对应的 页结构体 填上真实的物理地址等信息
        p = z->pages_group;
    
        for(j = 0;j < z->pages_length; j++ , p++)
        {
            // 每个物理页 属于 哪个 区域
            p->zone_struct = z;
            // start:0x200000 这是从物理地址0x200000开始分配可用2MB物理页
            p->PHY_address = start + PAGE_2M_SIZE * j;
            p->attribute = 0;

            p->reference_count = 0;

            p->age = 0;

            // 该物理页对应位图位置 请零 表示该可用物理页空闲
            *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;

        }
        
        
    }
    
    /*
    
        代码清单 4-72  
        物理地址的第一个2MB是 0x00 到0x1FFFFF 这里有BootLoader残留的机器码以及Kernel 对应Pages的第一个Page[0]
    
    */
    
    
    

    /////////////init address 0 to page struct 0; because the memory_management_struct.e820[0].type != 1
    
    memory_management_struct.pages_struct->zone_struct = memory_management_struct.zones_struct;

    memory_management_struct.pages_struct->PHY_address = 0UL;
    memory_management_struct.pages_struct->attribute = 0;
    memory_management_struct.pages_struct->reference_count = 0;
    memory_management_struct.pages_struct->age = 0;

    /////////////

    // 代码清单 4-70 的时候 假设有5个结果体 现在经过一系列配置 可以计算出真正结构体的数量 
    // 因为现在只有一个 zone 可用 所以其实 这里的 zones_size是1, 这里的 zones_length就是 0x50  50H=80D
    memory_management_struct.zones_length = (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & ( ~ (sizeof(long) - 1));
    //color_printk(WHITE,RED,"List4-72  zones_length:%#018lx\n", memory_management_struct.zones_length);


    /*
    
        代码清单 4-73 
    
    */


    color_printk(ORANGE,BLACK,"bits_map:%#018lx,bits_size:%#018lx,bits_length:%#018lx\n",memory_management_struct.bits_map,memory_management_struct.bits_size,memory_management_struct.bits_length);

    color_printk(ORANGE,BLACK,"pages_struct:%#018lx,pages_size:%#018lx,pages_length:%#018lx\n",memory_management_struct.pages_struct,memory_management_struct.pages_size,memory_management_struct.pages_length);

    color_printk(ORANGE,BLACK,"zones_struct:%#018lx,zones_size:%#018lx,zones_length:%#018lx\n",memory_management_struct.zones_struct,memory_management_struct.zones_size,memory_management_struct.zones_length);

    ZONE_DMA_INDEX = 0; //need rewrite in the future
    ZONE_NORMAL_INDEX = 0;  //need rewrite in the future



    /*
    
        代码清单 4-74
    
    */
    

    for(i = 0;i < memory_management_struct.zones_size;i++)  //need rewrite in the future
    {
        struct Zone * z = memory_management_struct.zones_struct + i;
        
        // 打印当前区域的 起始地址、结束地址、指向本区域的struct page结构体数组指针、区域内可用的物理页数量
        color_printk(ORANGE,BLACK,"zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n",z->zone_start_address,z->zone_end_address,z->zone_length,z->pages_group,z->pages_length);

        // 起始地址在0x1 0000 0000 的区域 未曾经过页表映射
        if(z->zone_start_address == 0x100000000)
            ZONE_UNMAPED_INDEX = i;
    }
    
    // end_of_struct 内存页管理结构的结尾地址
    // |--- 内核程序 --- | -----------      内存管理页结构                 ---------- |
    // |--- 内核程序 --- | ---- bits_map ---- | ---- pages ---- | ----- zones ---- |
    // end_of_struct = 0x12a150 预留一段内存空间防止越界访问
    // end_of_struct:0x000000000012a150 0x000000000012a050  
    memory_management_struct.end_of_struct = (unsigned long)((unsigned long)memory_management_struct.zones_struct + memory_management_struct.zones_length + sizeof(long) * 32) & ( ~ (sizeof(long) - 1));   ////need a blank to separate memory_management_struct

    
    
    /*
    
        代码清单4-75 初始化 struct Global_Memory_Descriptor 其中使用的结构体 struct Page
        结合代码清单4-79 理解 page_init
    
    */
    
    // end_of_struct 内存页管理结构的结尾地址
    color_printk(ORANGE,BLACK,"start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,end_of_struct:%#018lx\n",memory_management_struct.start_code,memory_management_struct.end_code,memory_management_struct.end_data,memory_management_struct.end_brk, memory_management_struct.end_of_struct);

    
    i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;


    // 将 系统内核 与 内存管理单元结构所占物理页的page结构体 全部初始化成
    // PG_PTable_Maped（经过页表映射的页） | PG_Kernel_Init（内核初始化程序） | PG_Active（使用中的页） | PG_Kernel（内核层页）属性
    for(j = 0;j <= i;j++)
    {
        page_init(memory_management_struct.pages_struct + j,PG_PTable_Maped | PG_Kernel_Init | PG_Active | PG_Kernel);
    }




    /*
    
        代码清单 4-76 清空页表项
    
    */


    Global_CR3 = Get_gdt();
    
    // PML4 - PDPT - PDT - PT

    color_printk(INDIGO,BLACK,"Global_CR3\t:%#018lx\n",Global_CR3);
    color_printk(INDIGO,BLACK,"*Global_CR3\t:%#018lx\n",*Phy_To_Virt(Global_CR3) & (~0xff));
    color_printk(PURPLE,BLACK,"**Global_CR3\t:%#018lx\n",*Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) & (~0xff));

    // 将PML4页表的前10个页表项清零
    // #define Phy_To_Virt(addr)    ((unsigned long *)((unsigned long)(addr) + PAGE_OFFSET))
    // PML4 单个页表项是8字节 这里强制转换成 unsigned long 类型的指针  +1 就是+8字节
    for(i = 0;i < 10;i++)
        *(Phy_To_Virt(Global_CR3)  + i) = 0UL;
    
    // 强制TLB 自动刷新
    flush_tlb();
}




/*

    number: number < 64

    zone_select: zone select from dma , mapped in  pagetable , unmapped in pagetable

    page_flags: struct Page flages

*/

struct Page * alloc_pages(int zone_select,int number,unsigned long page_flags)
{
    
    /*
    
        代码清单 4-80 
        目前，Bochs虚拟机只能开辟出2 GB的物理内存空间，以至于虚拟平台仅有一个可用物理内存段，
        因此ZONE_DMA_INDEX、ZONE_NORMAL_INDEX和ZONE_UNMAPED_INDEX 三个变量均代表同一内存区域空间，
        即使用默认值0代表的内存区域空间。
    
    */
    
    
    int i;
    unsigned long page = 0;

    int zone_start = 0;
    int zone_end = 0;

    switch(zone_select)
    {
        case ZONE_DMA:
                zone_start = 0;
                zone_end = ZONE_DMA_INDEX;

            break;

        case ZONE_NORMAL:
                zone_start = ZONE_DMA_INDEX;
                zone_end = ZONE_NORMAL_INDEX;

            break;

        case ZONE_UNMAPED:
                zone_start = ZONE_UNMAPED_INDEX;
                zone_end = memory_management_struct.zones_size - 1;

            break;

        default:
            color_printk(RED,BLACK,"alloc_pages error zone_select index\n");
            return NULL;
            break;
    }
    
    
    /*
    
        代码清单 4-81 从该区域空间中遍历出符合申请条件的struct page结构体组
        
    */
    
    for(i = zone_start;i <= zone_end; i++)
    {
        struct Zone * z;
        unsigned long j;
        unsigned long start,end,length;
        unsigned long tmp;

        if((memory_management_struct.zones_struct + i)->page_free_count < number)
            continue;

        z = memory_management_struct.zones_struct + i;
        start = z->zone_start_address >> PAGE_2M_SHIFT;
        end = z->zone_end_address >> PAGE_2M_SHIFT;
        length = z->zone_length >> PAGE_2M_SHIFT;

        
        // 起始页的BIT映射位图只能检索tmp = 64 - start % 64;次，
        // 即起始页只有 tmp 个位置可以检索 对应 tmp个物理页
        // 随后借助代码j += j % 64 ? tmp : 64将索引变量j调整到对齐位置处
        // 这里的64应该是是指 unsigned long 变量的8字节 64位
        // 也就是说 表示 bits_map的第一个 unsigned long 变量表示第0~63的物理页，里面有 tmp个位置可以检索
        // 下一次要走到第二个unsinged long 变量，表示第 64~127 的物理页
        /*
            start = 1   tmp = 64-1%64=63    j+= 1%64?63:64  j=1+63=64
            start = 2   tmp = 64-2%64=62    j+= 2%64?62:64  j=2+62=64
            start = 3   tmp = 64-3%64=61    j+= 3%64?61:64  j=3+61=64
        
        */

        tmp = 64 - start % 64;


        // 程序4-8的 start = 0x01 end=0x1023
        // 表示要从 [start,end]这么多个物理页中找一个区间，这个区间是64个连续的物理页
        // 程序4-8 可以直接从start开始 就能找到 连续64个物理页
        // 事实上，第一个找到的物理页的物理地址就是 0x200000
        // 外层for循环实际只会执行一次
        for(j = start;j <= end;j += j % 64 ? tmp : 64)
        {
            // j >> 6 就是 j / 64 看看当前检索的页对应的bit位在第几个unsigned long 变量里
            unsigned long * p = memory_management_struct.bits_map + (j >> 6);
            // shift 当前检索的页在该 unsigned long 变量中的偏移量
            // Hex:      0   0      0   0       0   0   0   0       0   0   0   0       0   0    0   0  
            // Binary: 0000 0000 0000 0000  0000 0000 0000 0000  0000 0000 0000 0000  0000 0000 0000 0000 
            unsigned long shift = j % 64;
            
            // k是下标，表示从当前的unsigned long变量的二进制第shift位开始检索，包括下一个unsigned long变量在内
            // 能不能够凑出来一个连续的64个物理页的区间
            // 也就是看 当前的unsigned long 变量的第shift位到下一个unsigned long变量的第shift-1位，是不是一直都是0
            // 0表示空闲，表示可以用
            // 如果是就申请这个区间
            // 如果不是k就要++
            unsigned long k;
            for(k = shift;k < 64 - shift;k++)
            {
                // 除非可以保证能申请到连续的64个物理页
                // 否则不会进入循环
                // 就是要一口气申请连续的64个物理页
                if( !(((*p >> k) | (*(p + 1) << (64 - k))) & (number == 64 ? 0xffffffffffffffffUL : ((1UL << number) - 1))) )
                {
                    
                    unsigned long   l;
                    page = j + k - 1;
                    
                    // page 表示相对于 整个 pages_struct的偏移
                    // l 表示 要申请的64个物理页的 偏移
                    for(l = 0;l < number;l++)
                    {
                        struct Page * x = memory_management_struct.pages_struct + page + l;
                        // page_init函数负责对位图置位，设置页结构的属性等等
                        page_init(x,page_flags);
                    }
                    // 上面的for一口气把64个物理页申请好了 因此这里的page仍旧是第一个申请的物理页的偏移
                    goto find_free_pages;
                }
            }
        
        }
    }

    return NULL;

find_free_pages:

    return (struct Page *)(memory_management_struct.pages_struct + page);
}
