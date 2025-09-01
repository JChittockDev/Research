using Microsoft.EntityFrameworkCore;
using System.Globalization;
using CanadaWalksAPI.Mappings;

var builder = WebApplication.CreateBuilder(args);

Console.WriteLine($"CurrentCulture: {CultureInfo.CurrentCulture.Name}");
Console.WriteLine($"CurrentUICulture: {CultureInfo.CurrentUICulture.Name}");
Console.WriteLine($"Invariant Mode: {System.Runtime.InteropServices.RuntimeInformation.FrameworkDescription}");

// Add services to the container.

builder.Services.AddControllers();
// Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

builder.Services.AddDbContext<CanadaWalksAPI.Data.CanadaWalksDbContext>(options => options.UseSqlServer(builder.Configuration.GetConnectionString("CanadaWalksConnectionString")));
builder.Services.AddScoped<CanadaWalksAPI.Repositories.IRegionRepository, CanadaWalksAPI.Repositories.SQLRegionRepository>();
builder.Services.AddScoped<CanadaWalksAPI.Repositories.IWalkRepository, CanadaWalksAPI.Repositories.SQLWalkRepository>();

builder.Services.AddAutoMapper(typeof(AutoMapperProfiles));

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseHttpsRedirection();

app.UseAuthorization();

app.MapControllers();

app.Run();
