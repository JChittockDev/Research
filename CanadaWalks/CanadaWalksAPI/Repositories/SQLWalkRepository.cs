using CanadaWalksAPI.Data;
using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.RTO;
using Microsoft.EntityFrameworkCore;

namespace CanadaWalksAPI.Repositories
{
    public class SQLWalkRepository : IWalkRepository
    {
        // This class is intended to implement the IWalkRepository interface
        // and provide methods for managing Walks in a SQL database.
        private readonly CanadaWalksDbContext dbContext;

        // Constructor that accepts CanadaWalksDbContext
        public SQLWalkRepository(CanadaWalksDbContext dbContext)
        {
            this.dbContext = dbContext;
        }

        public async Task<Walk> AddWalkAsync(Walk Walk)
        {
            await dbContext.Walks.AddAsync(Walk);
            await dbContext.SaveChangesAsync();

            return Walk;
        }

        public async Task<bool?> DeleteWalkAsync(Guid id)
        {
            // Find the Walk by ID
            var Walk = await dbContext.Walks.FirstOrDefaultAsync(r => r.Id == id);

            // If the Walk is not found, return NotFound
            if (Walk == null)
            {
                return false;
            }

            // Remove the Walk from the database
            dbContext.Walks.Remove(Walk);
            await dbContext.SaveChangesAsync();

            return true;
        }

        public async Task<List<Walk>> GetAllWalksAsync()
        {
            // Include related entities (Difficulty and Region) using eager loading to avoid lazy loading issues (this is like a pointer in C/C++) 
            return await dbContext.Walks.Include("Difficulty").Include("Region").ToListAsync();
        }

        public async Task<Walk?> GetWalkByIdAsync(Guid id)
        {
            return await dbContext.Walks.Include("Difficulty").Include("Region").FirstOrDefaultAsync(r => r.Id == id);
        }

        public async Task<Walk?> UpdateWalkAsync(Guid id, UpdateWalkRTO Walk)
        {
            // Find the existing Walk by ID
            var foundWalk = await dbContext.Walks.FirstOrDefaultAsync(r => r.Id == id);

            // If the Walk is not found, return NotFound
            if (foundWalk == null)
            {
                return null;
            }

            if (Walk.Name != null)
            {
                foundWalk.Name = Walk.Name;
            }

            if (Walk.Description != null)
            {
                foundWalk.Description = Walk.Description;
            }

            if (Walk.Length != null)
            {
                foundWalk.Length = Walk.Length.Value;
            }

            if (Walk.WalkImageUrl != null)
            {
                foundWalk.WalkImageUrl = Walk.WalkImageUrl;
            }

            if (Walk.RegionId != null)
            {
                foundWalk.RegionId = Walk.RegionId.Value;
            }

            if (Walk.DifficultyId != null)
            {
                foundWalk.DifficultyId = Walk.DifficultyId.Value;
            }

            await dbContext.SaveChangesAsync();

            return foundWalk;
        }
    }
}
