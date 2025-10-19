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

        public async Task<List<Walk>> GetAllWalksAsync(
            string? filterOn = null, 
            string? filterQuery = null, 
            string? sortBy = null, 
            bool isAscending = true,
            int pageNumber = 1,
            int pageSize = 1000)
        {
            // Include related entities (Difficulty and Region) using eager loading to avoid lazy loading issues (this is like a pointer in C/C++) 
            var walks = dbContext.Walks.Include("Difficulty").Include("Region").AsQueryable();

            // Filtering
            // Check if filterOn and filterQuery are provided
            if (!string.IsNullOrWhiteSpace(filterOn) && !string.IsNullOrWhiteSpace(filterQuery))
            {
                // This handles the case where filterOn is "Name"
                if (filterOn.Equals("Name"))
                {
                    walks = walks.Where(x => x.Name.Contains(filterQuery));
                }
                else if (filterOn.Equals("Description"))
                {
                    walks = walks.Where(x => x.Description.Contains(filterQuery));
                }
                else if (filterOn.Equals("Region"))
                {
                    walks = walks.Where(x => x.Region.Name.Contains(filterQuery));
                }
                else if (filterOn.Equals("Difficulty"))
                {
                    walks = walks.Where(x => x.Difficulty.Name.Contains(filterQuery));
                }
                else if (filterOn.Equals("Length") && double.TryParse(filterQuery, out double length))
                {
                    // Exact match for Length
                    walks = walks.Where(x => x.Length == length);
                }
            }
            
            // Sorting
            if (!string.IsNullOrWhiteSpace(sortBy))
            {
                if (sortBy.Equals("Name"))
                {
                    walks = isAscending ? walks.OrderBy(x => x.Name) : walks.OrderByDescending(x => x.Name);
                }
                else if (sortBy.Equals("Length"))
                {
                    walks = isAscending ? walks.OrderBy(x => x.Length) : walks.OrderByDescending(x => x.Length);
                }
                else if (sortBy.Equals("Region"))
                {
                    walks = isAscending ? walks.OrderBy(x => x.Region.Name) : walks.OrderByDescending(x => x.Region.Name);
                }
                else if (sortBy.Equals("Difficulty"))
                {
                    walks = isAscending ? walks.OrderBy(x => x.Difficulty.Name) : walks.OrderByDescending(x => x.Difficulty.Name);
                }
            }

            // Pagination
            // Calculate the number of records to skip based on the page number and page size
            var skipResults = (pageNumber - 1) * pageSize;

            // Apply pagination using Skip and Take
            return await walks.Skip(skipResults).Take(pageSize).ToListAsync();
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
