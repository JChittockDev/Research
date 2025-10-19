using CanadaWalksAPI.Data;
using CanadaWalksAPI.Models.Domain;
using Microsoft.EntityFrameworkCore;

namespace CanadaWalksAPI.Repositories
{
    public class ImageRepository : IImageRepository
    {
        private readonly IWebHostEnvironment webHostEnvironment;
        private readonly IHttpContextAccessor httpContextAccessor;
        private readonly CanadaWalksDbContext canadaWalksDbContext;

        public ImageRepository(IWebHostEnvironment webHostEnvironment, IHttpContextAccessor httpContextAccessor, CanadaWalksDbContext canadaWalksDbContext) 
        {
            this.webHostEnvironment = webHostEnvironment;
            this.httpContextAccessor = httpContextAccessor;
            this.canadaWalksDbContext = canadaWalksDbContext;
        }
        public async Task<bool?> DeleteImageByIdAsync(Guid id)
        {
            var image = await canadaWalksDbContext.Images.FindAsync(id);
            if (image == null)
            {
                return false;
            }

            // Delete the image file from the local file system
            var localFilePath = Path.Combine(webHostEnvironment.ContentRootPath, "Images", $"{image.FileName}{image.FileType}");
            if (File.Exists(localFilePath))
            {
                File.Delete(localFilePath);
            }
            
            // Remove the image record from the database
            canadaWalksDbContext.Images.Remove(image);
            await canadaWalksDbContext.SaveChangesAsync();
            return true;
        }

        public async Task<bool?> DeleteImageByNameAsync(string name)
        {
            var image = await canadaWalksDbContext.Images.FirstOrDefaultAsync(x => x.FileName == name);
            if (image == null)
            {
                return false;
            }
            // Delete the image file from the local file system
            var localFilePath = Path.Combine(webHostEnvironment.ContentRootPath, "Images", $"{image.FileName}{image.FileType}");
            if (File.Exists(localFilePath))
            {
                File.Delete(localFilePath);
            }
            // Remove the image record from the database
            canadaWalksDbContext.Images.Remove(image);
            await canadaWalksDbContext.SaveChangesAsync();
            return true;
        }

        public async Task<bool?> DeleteImageByPathAsync(string path)
        {
            var image = await canadaWalksDbContext.Images.FirstOrDefaultAsync(x => x.FilePath == path);
            if (image == null)
            {
                return false;
            }
            // Delete the image file from the local file system
            var localFilePath = Path.Combine(webHostEnvironment.ContentRootPath, "Images", $"{image.FileName}{image.FileType}");
            if (File.Exists(localFilePath))
            {
                File.Delete(localFilePath);
            }
            // Remove the image record from the database
            canadaWalksDbContext.Images.Remove(image);
            await canadaWalksDbContext.SaveChangesAsync();
            return true;
        }

        public async Task<Image> GetImageByIdAsync(Guid id)
        {
            
            return await canadaWalksDbContext.Images.FindAsync(id);
        }

        public async Task<Image> GetImageByNameAsync(string name)
        {
            return await canadaWalksDbContext.Images.FirstOrDefaultAsync(x => x.FileName == name);
        }

        public async Task<Image> GetImageByPathAsync(string path)
        {
            return await canadaWalksDbContext.Images.FirstOrDefaultAsync(x => x.FilePath == path);
        }

        public async Task<Image> UploadImageAsync(Image image)
        {
            var localFilePath = Path.Combine(webHostEnvironment.ContentRootPath, "Images", $"{image.FileName}{image.FileType}");

            // Create directory if it doesn't exist
            using var stream = new FileStream(localFilePath, FileMode.Create);
            // Save the file to the local file system
            await image.File.CopyToAsync(stream);

            // Return the URL of the uploaded image
            // http://localhost:7120/Images/filename.jpg
            var urlFilePath = $"{httpContextAccessor.HttpContext.Request.Scheme}://{httpContextAccessor.HttpContext.Request.Host}{httpContextAccessor.HttpContext.Request.PathBase}/Images/{image.FileName}{image.FileType}";

            image.FilePath = urlFilePath;

            await canadaWalksDbContext.Images.AddAsync(image);
            await canadaWalksDbContext.SaveChangesAsync();

            return image;

        }
    }
}
