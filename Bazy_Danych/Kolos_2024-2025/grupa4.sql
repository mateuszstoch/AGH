--zadanie 1 
select m.member_no,m.firstname ,m.lastname from [member] m 
left join loanhist l on year(l.out_date) = 2001 and m.member_no = l.member_no 
group by m.member_no,m.firstname ,m.lastname 
having  count(l.out_date) = 0
-- mozna dodac ten czlon ale nie ma zadnych wypozyczen w 2001 roku
union
select m.member_no,m.firstname ,m.lastname from [member] m 
left join loan l on year(l.out_date) = 2001 and m.member_no = l.member_no 
group by m.member_no,m.firstname ,m.lastname 
having  count(l.out_date) = 0

-- zapytanie sprawdzajace (negatyw pokazuje kto wypozyczyl)
select l.member_no , count(*) from loanhist l where year(l.out_date) = 2001 group by l.member_no 
union 
select l.member_no , count(*) from loan l where year(l.out_date) = 2001 group by l.member_no 

--zadanie 2
select top 1 e.FirstName, e.LastName,round(sum(od.Quantity * od.UnitPrice * (1-od.Discount) )+ sum(distinct o.Freight),2) from Employees e 
join Orders o on e.EmployeeID = o.EmployeeID and year(o.OrderDate ) = 1997
join [Order Details] od on o.OrderId = od.OrderID 
group by e.FirstName, e.LastName,e.EmployeeID 
order by 3 desc

-- zadanie 3
select c.CompanyName, count(*) from Customers c 
join Orders o on o.CustomerID = c.CustomerID and year(o.OrderDate) = 1997
join [Order Details] od on o.OrderID = od.OrderID
join Products p on p.ProductID = od.ProductID 
join Categories c2 on p.CategoryID = c2.CategoryID and c2.CategoryName = 'Confections'
group by c.CompanyName, c.CustomerID 
having count(*) >= 2

--zapytanie sprawdzajace 
select c.CustomerID ,c2.CategoryName, count(*) from Orders o 
join Customers c on o.CustomerID = c.CustomerID 
join [Order Details] od on o.OrderID = od.OrderID 
join Products p on p.ProductID = od.ProductID 
join Categories c2 on p.CategoryID = c2.CategoryID
where c.CompanyName = 'Antonio Moreno Taquería' and year(o.OrderDate) = 1997
group by c.CustomerID ,c2.CategoryName