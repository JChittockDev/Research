using Microsoft.EntityFrameworkCore;
using CanadaWalksAPI.Models.Domain;

namespace CanadaWalksAPI.Data
{
    public class CanadaWalksDbContext: DbContext
    {
        // Constructor that accepts DbContextOptions and passes it to the base class
        public CanadaWalksDbContext(DbContextOptions<CanadaWalksDbContext> dbContextOptions): base(dbContextOptions)
        {
            
        }

        // DbSet properties for each entity type
        public DbSet<Region> Regions { get; set; }
        public DbSet<Walk> Walks { get; set; }
        public DbSet<Difficulty> Difficulties { get; set; }
        public DbSet<Image> Images { get; set; }

        // Override OnModelCreating to configure the model
        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            base.OnModelCreating(modelBuilder);

            // Seed(Push/Populate) difficulty data for Regions
            var difficulties = new List<Difficulty>()
            {
                new Difficulty()
                {
                    Id = Guid.Parse("a2f5e1b6-1c3b-4d5e-8f7a-9b0c1d2e3f40"),
                    Name = "Easy"
                },
                new Difficulty()
                {
                    Id = Guid.Parse("c4d5e6f7-8a9b-4c1d-2e3f-4a5b6c7d8e9f"),
                    Name = "Medium"
                },
                new Difficulty()
                {
                    Id = Guid.Parse("b3f6a2c7-2d4e-5f6a-9b0a-1b2c3d4e5f60"),
                    Name = "Hard"
                }
            };

            modelBuilder.Entity<Difficulty>().HasData(difficulties);

            var regions = new List<Region>()
            {
                new Region()
                {
                    Id = Guid.Parse("d1e2f3a4-b5c6-7d8e-9f0a-1b2c3d4e5f60"),
                    Name = "British Columbia",
                    Code = "BC",
                    RegionImageUrl = "https://example.com/images/bc.jpg",
                },
                new Region()
                {
                    Id = Guid.Parse("e2f3a4b5-c6d7-8e9f-0a1b-2c3d4e5f6a7b"),
                    Name = "Alberta",
                    Code = "AB",
                    RegionImageUrl = "https://example.com/images/ab.jpg"
                },
                new Region()
                {
                    Id = Guid.Parse("f3a4b5c6-d7e8-9f0a-1b2c-3d4e5f6a7b8c"),
                    Name = "Ontario",
                    Code = "ON",
                    RegionImageUrl = "https://example.com/images/on.jpg"
                },
                new Region()
                {
                    Id = Guid.Parse("a4b5c6d7-e8f9-0a1b-2c3d-4e5f6a7b8c9d"),
                    Name = "Quebec",
                    Code = "QC",
                    RegionImageUrl = "https://example.com/images/qc.jpg"
                }
            };

            // Seed the regions data
            modelBuilder.Entity<Region>().HasData(regions);
        }
    }
}
