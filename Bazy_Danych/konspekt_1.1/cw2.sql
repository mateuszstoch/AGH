-- zadanie 1
select * from Customers c where c.CompanyName like '%Restaurant%'

-- zadanie 2
select * from Products p where p.QuantityPerUnit like '%bottle%'

-- zadanie 3
select e.Title from Employees e where e.LastName like '[B-L]%'

-- zadanie 4
select e.Title  from Employees e where e.LastName like '[B,L]%'

-- zadanie 5
select * from Categories c where c.Description like '%,%'

-- zadanie 6
select * from Customers c where c.CompanyName like '%store%'