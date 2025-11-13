-- zadanie 1
select c.Country ,c.CompanyName  from Customers c order by c.Country ,c.CompanyName 

-- zadanie 2
select c.country, c.CompanyName  from Customers c 
where c.Country  in ('France','spain') 
order by c.Country , c.CompanyName 

-- zadanie 3
select * from Orders o where year(o.OrderDate) = 1997 order by Month(o.OrderDate )desc , o.Freight