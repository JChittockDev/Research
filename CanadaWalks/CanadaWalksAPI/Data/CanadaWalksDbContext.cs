using Microsoft.EntityFrameworkCore;

namespace CanadaWalksAPI.Data
{
    public class CanadaWalksDbContext: DbContext
    {
        // Constructor that accepts DbContextOptions and passes it to the base class
        public CanadaWalksDbContext(DbContextOptions dbContextOptions): base(dbContextOptions)
        {
            
        }

        // DbSet properties for each entity type
        public DbSet<Models.Domain.Region> Regions { get; set; }
        public DbSet<Models.Domain.Walk> Walks { get; set; }
        public DbSet<Models.Domain.Difficulty> Difficulties { get; set; }


    }
}
