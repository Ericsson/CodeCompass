using System.Text;
using Microsoft.EntityFrameworkCore;

namespace StandAloneCSharpParser.model
{
    class CsharpDbContext : DbContext
    {

        string ConnenctionString;
        public CsharpDbContext(string connectionString){
            ConnenctionString = connectionString;
        }
        
        public DbSet<CsharpAstNode> CsharpAstNodes { get; set; }
        public DbSet<CsharpNamespace> CsharpNamespaces { get; set; }
        public DbSet<CsharpClass> CsharpClasses { get; set; }
        public DbSet<CsharpStruct> CsharpStructs { get; set; }
        public DbSet<CsharpEnum> CsharpEnums { get; set; }

        protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder)
            => optionsBuilder.UseNpgsql(ConnenctionString);
           // => optionsBuilder.UseNpgsql("Host=localhost;Database=postgres;Username=compass;Password=1234");

    }

}
