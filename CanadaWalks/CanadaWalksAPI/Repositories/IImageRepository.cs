namespace CanadaWalksAPI.Repositories
{
    public interface IImageRepository
    {
        Task<Models.Domain.Image> UploadImageAsync(Models.Domain.Image image);
        Task<Models.Domain.Image> GetImageByIdAsync(Guid id);
        Task<Models.Domain.Image> GetImageByNameAsync(string name);
        Task<Models.Domain.Image> GetImageByPathAsync(string name);
        Task<bool?> DeleteImageByIdAsync(Guid id);
        Task<bool?> DeleteImageByNameAsync(string name);
        Task<bool?> DeleteImageByPathAsync(string path);
    }
}
