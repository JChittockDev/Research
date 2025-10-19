using Microsoft.AspNetCore.Identity;
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;

namespace CanadaWalksAPI.Data
{
    public class CanadaWalksAuthDbContext : IdentityDbContext
    {
        public CanadaWalksAuthDbContext(DbContextOptions<CanadaWalksAuthDbContext> options) : base(options)
        {
        }

        protected override void OnModelCreating(ModelBuilder builder)
        {
            base.OnModelCreating(builder);

            var adminRoleId = "70385ade-02da-4a06-81de-0b2f9687f8d1";
            var userRoleId = "95f06703-62cb-426e-9129-bb8f65a5c54c";

            var roles = new List<IdentityRole>()
            {
                new IdentityRole()
                {
                    Id = adminRoleId,
                    ConcurrencyStamp = adminRoleId,
                    Name = "Admin",
                    NormalizedName = "ADMIN"
                },
                new IdentityRole()
                {
                    Id = userRoleId,
                    ConcurrencyStamp = userRoleId,
                    Name = "User",
                    NormalizedName = "USER"
                }
            };

            builder.Entity<IdentityRole>().HasData(roles);
        }
    }
}
