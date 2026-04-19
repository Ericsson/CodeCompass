using System.Text;
using Microsoft.EntityFrameworkCore;

namespace DbModel
{
  public class CsharpDbContext : DbContext
  {

    private readonly string _dbSystem;
    private readonly string _connectionString;
    public CsharpDbContext(DbContextOptions options) : base(options) { }

    public CsharpDbContext(string dbSystem, string connectionString) : base()
    {
        _dbSystem = dbSystem;
        _connectionString = connectionString;
    }

    public DbSet<CsharpAstNode> CsharpAstNodes { get; set; }
    public DbSet<CsharpNamespace> CsharpNamespaces { get; set; }
    public DbSet<CsharpClass> CsharpClasses { get; set; }
    public DbSet<CsharpMethod> CsharpMethods { get; set; }
    public DbSet<CsharpVariable> CsharpVariables { get; set; }
    public DbSet<CsharpStruct> CsharpStructs { get; set; }
    public DbSet<CsharpEnum> CsharpEnums { get; set; }
    public DbSet<CsharpEnumMember> CsharpEnumMembers { get; set; }
    public DbSet<CsharpEtcEntity> CsharpEtcEntitys { get; set; }

    protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder)
    {
        switch(_dbSystem)
        {
            case "pgsql":
                optionsBuilder.UseNpgsql(_connectionString,
                x => x.MigrationsAssembly("pgsqlMigrations"));
                break;
            case "sqlite":
                optionsBuilder.UseSqlite(_connectionString,
                x => x.MigrationsAssembly("sqliteMigrations"));
                break;
            default:
                break;
        }
    }

  }

}
