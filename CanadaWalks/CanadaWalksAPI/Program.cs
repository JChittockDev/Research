using Microsoft.EntityFrameworkCore;
using System.Globalization;
using CanadaWalksAPI.Mappings;
using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.IdentityModel.Tokens;
using System.Text;
using Microsoft.AspNetCore.Identity;

var builder = WebApplication.CreateBuilder(args);

Console.WriteLine($"CurrentCulture: {CultureInfo.CurrentCulture.Name}");
Console.WriteLine($"CurrentUICulture: {CultureInfo.CurrentUICulture.Name}");
Console.WriteLine($"Invariant Mode: {System.Runtime.InteropServices.RuntimeInformation.FrameworkDescription}");

// Add services to the container.

builder.Services.AddControllers();
builder.Services.AddHttpContextAccessor();
// Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
builder.Services.AddEndpointsApiExplorer();
// Configure Swagger to use JWT Authentication
builder.Services.AddSwaggerGen(options =>
{
    options.SwaggerDoc("v1", new Microsoft.OpenApi.Models.OpenApiInfo
    {
        Title = "Canada Walks API",
        Version = "v1"
    });
    
    options.AddSecurityDefinition(JwtBearerDefaults.AuthenticationScheme, new Microsoft.OpenApi.Models.OpenApiSecurityScheme
    {
        Name = "Authorization",
        In = Microsoft.OpenApi.Models.ParameterLocation.Header,
        Type = Microsoft.OpenApi.Models.SecuritySchemeType.ApiKey,
        Scheme = JwtBearerDefaults.AuthenticationScheme,
    });
    
    options.AddSecurityRequirement(new Microsoft.OpenApi.Models.OpenApiSecurityRequirement
    {
        {
            new Microsoft.OpenApi.Models.OpenApiSecurityScheme
            {
                Reference = new Microsoft.OpenApi.Models.OpenApiReference
                {
                    Type = Microsoft.OpenApi.Models.ReferenceType.SecurityScheme,
                    Id = JwtBearerDefaults.AuthenticationScheme
                },
                Scheme = "Oauth2",
                Name = JwtBearerDefaults.AuthenticationScheme,
                In = Microsoft.OpenApi.Models.ParameterLocation.Header
            },
            new List<string>()
        }
    });
});

builder.Services.AddDbContext<CanadaWalksAPI.Data.CanadaWalksDbContext>(options => options.UseSqlServer(builder.Configuration.GetConnectionString("CanadaWalksConnectionString")));
builder.Services.AddDbContext<CanadaWalksAPI.Data.CanadaWalksAuthDbContext>(options => options.UseSqlServer(builder.Configuration.GetConnectionString("CanadaWalksAuthConnectionString")));

builder.Services.AddScoped<CanadaWalksAPI.Repositories.IRegionRepository, CanadaWalksAPI.Repositories.SQLRegionRepository>();
builder.Services.AddScoped<CanadaWalksAPI.Repositories.IWalkRepository, CanadaWalksAPI.Repositories.SQLWalkRepository>();
builder.Services.AddScoped<CanadaWalksAPI.Repositories.ITokenRepository, CanadaWalksAPI.Repositories.TokenRepository>();
builder.Services.AddScoped<CanadaWalksAPI.Repositories.IImageRepository, CanadaWalksAPI.Repositories.ImageRepository>();

builder.Services.AddAutoMapper(typeof(AutoMapperProfiles));

// Configure Identity and Authentication
builder.Services.AddIdentityCore<IdentityUser>()
    .AddRoles<IdentityRole>()
    .AddTokenProvider<DataProtectorTokenProvider<IdentityUser>>("CanadaWalks")
    .AddEntityFrameworkStores<CanadaWalksAPI.Data.CanadaWalksAuthDbContext>()
    .AddDefaultTokenProviders();

// Configure Identity options
builder.Services.Configure<IdentityOptions>(options =>
{
    options.Password.RequireDigit = true;
    options.Password.RequireLowercase = true;
    options.Password.RequireUppercase = true;
    options.Password.RequireNonAlphanumeric = false;
    options.Password.RequiredLength = 8;
    options.User.RequireUniqueEmail = true;
});

// Configure JWT Authentication
builder.Services.AddAuthentication(JwtBearerDefaults.AuthenticationScheme)
.AddJwtBearer( options => options.TokenValidationParameters = new TokenValidationParameters
{
    // This section validates the JWT
    ValidateIssuer = true,
    // This section validates the Audience of the JWT
    ValidateAudience = true,
    // This section validates the lifetime of the token
    ValidateLifetime = true,
    // This section validates the signing key of the token
    ValidateIssuerSigningKey = true,
    // These are the actual values that will be used to validate the token
    ValidIssuer = builder.Configuration["Jwt:Issuer"],
    // These audience is the intended recipient of the token
    ValidAudience = builder.Configuration["Jwt:Audience"],
    // This is the secret key used to sign the token
    IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(builder.Configuration["Jwt:Key"]))
});

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseHttpsRedirection();

app.UseAuthentication();

app.UseAuthorization();

app.UseStaticFiles(new StaticFileOptions
{
    FileProvider = new Microsoft.Extensions.FileProviders.PhysicalFileProvider(Path.Combine(Directory.GetCurrentDirectory(), "Images")),
    RequestPath = "/Images"
});

app.MapControllers();

app.Run();
